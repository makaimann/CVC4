/*********************                                                        */
/*! \file model_engine.cpp
 ** \verbatim
 ** Original author: Morgan Deters
 ** Major contributors: Andrew Reynolds
 ** Minor contributors (to current version): Francois Bobot
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2014  New York University and The University of Iowa
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Implementation of model engine class
 **/

#include "theory/quantifiers/model_engine.h"
#include "theory/theory_engine.h"
#include "theory/uf/equality_engine.h"
#include "theory/uf/theory_uf.h"
#include "theory/uf/theory_uf_strong_solver.h"
#include "theory/quantifiers/options.h"
#include "theory/quantifiers/first_order_model.h"
#include "theory/quantifiers/term_database.h"
#include "theory/quantifiers/quantifiers_attributes.h"
#include "theory/quantifiers/full_model_check.h"
#include "theory/quantifiers/ambqi_builder.h"

using namespace std;
using namespace CVC4;
using namespace CVC4::kind;
using namespace CVC4::context;
using namespace CVC4::theory;
using namespace CVC4::theory::quantifiers;
using namespace CVC4::theory::inst;

//Model Engine constructor
ModelEngine::ModelEngine( context::Context* c, QuantifiersEngine* qe ) :
QuantifiersModule( qe ),
d_incomplete_check(false),
d_addedLemmas(0),
d_triedLemmas(0),
d_totalLemmas(0)
{

}

ModelEngine::~ModelEngine() {

}

bool ModelEngine::needsCheck( Theory::Effort e ) {
  return e==Theory::EFFORT_LAST_CALL;
}

unsigned ModelEngine::needsModel( Theory::Effort e ) {
  return QuantifiersEngine::QEFFORT_MODEL;  
}

void ModelEngine::check( Theory::Effort e, unsigned quant_e ){
  if( quant_e==QuantifiersEngine::QEFFORT_MODEL ){
    int addedLemmas = 0;
    FirstOrderModel* fm = d_quantEngine->getModel();

    //the following will test that the model satisfies all asserted universal quantifiers by 
    // (model-based) exhaustive instantiation.
    double clSet = 0;
    if( Trace.isOn("model-engine") ){
      Trace("model-engine") << "---Model Engine Round---" << std::endl;
      clSet = double(clock())/double(CLOCKS_PER_SEC);
    }
    ++(d_statistics.d_inst_rounds);

    Trace("model-engine-debug") << "Verify uf ss is minimal..." << std::endl;
    //let the strong solver verify that the model is minimal
    //for debugging, this will if there are terms in the model that the strong solver was not notified of
    uf::StrongSolverTheoryUF * ufss = ((uf::TheoryUF*)d_quantEngine->getTheoryEngine()->theoryOf( THEORY_UF ))->getStrongSolver();
    if( !ufss || ufss->debugModel( fm ) ){
      Trace("model-engine-debug") << "Check model..." << std::endl;
      d_incomplete_check = false;
      //print debug
      Trace("fmf-model-complete") << std::endl;
      debugPrint("fmf-model-complete");
      //successfully built an acceptable model, now check it
      addedLemmas += checkModel();
    }else{
      addedLemmas++;
    }
      
    if( Trace.isOn("model-engine") ){
      double clSet2 = double(clock())/double(CLOCKS_PER_SEC);
      Trace("model-engine") << "Finished model engine, time = " << (clSet2-clSet) << std::endl;
    }

    if( addedLemmas==0 ){
      Trace("model-engine-debug") << "No lemmas added, incomplete = " << d_incomplete_check << std::endl;
      //CVC4 will answer SAT or unknown
      Trace("fmf-consistent") << std::endl;
      debugPrint("fmf-consistent");
      //if the check was incomplete, we must set incomplete flag
      if( d_incomplete_check ){
        d_quantEngine->getOutputChannel().setIncomplete();
      }
    }else{
      //otherwise, the search will continue
    }
  }
}

void ModelEngine::registerQuantifier( Node f ){
  if( Trace.isOn("fmf-warn") ){
    bool canHandle = true;
    for( unsigned i=0; i<f[0].getNumChildren(); i++ ){
      TypeNode tn = f[0][i].getType();
      if( !tn.isSort() ){
        if( !tn.getCardinality().isFinite() ){
          if( tn.isInteger() ){
            if( !options::fmfBoundInt() ){
              canHandle = false;
            }
          }else{
            canHandle = false;
          }
        }
      }
    }
    if( !canHandle ){
      Trace("fmf-warn") << "Warning : Model Engine : may not be able to answer SAT because of formula : " << f << std::endl;
    }
  }
}

void ModelEngine::assertNode( Node f ){

}

bool ModelEngine::optOneQuantPerRound(){
  return options::fmfOneQuantPerRound();
}


int ModelEngine::checkModel(){
  FirstOrderModel* fm = d_quantEngine->getModel();
  quantifiers::QModelBuilder * mb = d_quantEngine->getModelBuilder();
  Assert( mb!=NULL );

  //flatten the representatives
  //Trace("model-engine-debug") << "Flattening representatives...." << std::endl;
  //d_quantEngine->getEqualityQuery()->flattenRepresentatives( fm->d_rep_set.d_type_reps );

  //for debugging
  if( Trace.isOn("model-engine") || Trace.isOn("model-engine-debug") ){
    for( std::map< TypeNode, std::vector< Node > >::iterator it = fm->d_rep_set.d_type_reps.begin();
         it != fm->d_rep_set.d_type_reps.end(); ++it ){
      if( it->first.isSort() ){
        Trace("model-engine") << "Cardinality( " << it->first << " )" << " = " << it->second.size() << std::endl;
        if( Trace.isOn("model-engine-debug") ){
          Trace("model-engine-debug") << "   ";
          Node mbt = d_quantEngine->getTermDatabase()->getModelBasisTerm(it->first);
          for( size_t i=0; i<it->second.size(); i++ ){
            //Trace("model-engine-debug") << it->second[i] << "  ";
            Node r = d_quantEngine->getEqualityQuery()->getInternalRepresentative( it->second[i], Node::null(), 0 );
            Trace("model-engine-debug") << r << " ";
          }
          Trace("model-engine-debug") << std::endl;
          Trace("model-engine-debug") << "  Model basis term : " << mbt << std::endl;
        }
      }
    }
  }

  d_triedLemmas = 0;
  d_addedLemmas = 0;
  d_totalLemmas = 0;
  //for statistics
  for( int i=0; i<fm->getNumAssertedQuantifiers(); i++ ){
    Node f = fm->getAssertedQuantifier( i );
    int totalInst = 1;
    for( size_t i=0; i<f[0].getNumChildren(); i++ ){
      TypeNode tn = f[0][i].getType();
      if( fm->d_rep_set.hasType( tn ) ){
        totalInst = totalInst * (int)fm->d_rep_set.d_type_reps[ tn ].size();
      }
    }
    d_totalLemmas += totalInst;
  }

  Trace("model-engine-debug") << "Do exhaustive instantiation..." << std::endl;
  //default : 1 : conj,axiom
  //priority : 0 : conj, 1 : axiom
  //trust : 0 : conj
  int startEffort = ( !fm->isAxiomAsserted() || options::axiomInstMode()==AXIOM_INST_MODE_DEFAULT ) ? 1 : 0;
  for( int effort=startEffort; effort<2; effort++ ){
    // FMC uses two sub-effort levels
    int e_max = options::mbqiMode()==MBQI_FMC || options::mbqiMode()==MBQI_FMC_INTERVAL ? 2 : ( options::mbqiMode()==MBQI_TRUST ? 0 : 1 );
    for( int e=0; e<e_max; e++) {
      if (d_addedLemmas==0) {
        for( int i=0; i<fm->getNumAssertedQuantifiers(); i++ ){
          Node f = fm->getAssertedQuantifier( i );
          bool isAx = getTermDatabase()->isQAttrAxiom( f );
          //determine if we should check this quantifier
          if( ( ( effort==1 && ( options::axiomInstMode()==AXIOM_INST_MODE_DEFAULT || isAx ) ) || ( effort==0 && !isAx ) ) && mb->isQuantifierActive( f ) ){
            exhaustiveInstantiate( f, e );
            if( Trace.isOn("model-engine-warn") ){
              if( d_addedLemmas>10000 ){
                Debug("fmf-exit") << std::endl;
                debugPrint("fmf-exit");
                exit( 0 );
              }
            }
            if( optOneQuantPerRound() && d_addedLemmas>0 ){
              break;
            }
          }else{
            Trace("inst-fmf-ei") << "-> Inactive : " << f << std::endl;
          }
        }
      }
    }
    if( d_addedLemmas==0 && options::axiomInstMode()==AXIOM_INST_MODE_TRUST ){
      //set incomplete
      if( effort==0 ){
        d_quantEngine->getOutputChannel().setIncomplete();
      }
      break;
    }
  }
  //print debug information
  Trace("model-engine") << "Added Lemmas = " << d_addedLemmas << " / " << d_triedLemmas << " / ";
  Trace("model-engine") << d_totalLemmas << std::endl;
  return d_addedLemmas;
}

void ModelEngine::exhaustiveInstantiate( Node f, int effort ){
  //first check if the builder can do the exhaustive instantiation
  quantifiers::QModelBuilder * mb = d_quantEngine->getModelBuilder();
  mb->d_triedLemmas = 0;
  mb->d_addedLemmas = 0;
  mb->d_incomplete_check = false;
  if( mb->doExhaustiveInstantiation( d_quantEngine->getModel(), f, effort ) ){
    Trace("inst-fmf-ei") << "-> Builder determined instantiation(s)." << std::endl;
    d_triedLemmas += mb->d_triedLemmas;
    d_addedLemmas += mb->d_addedLemmas;
    d_incomplete_check = d_incomplete_check || mb->d_incomplete_check;
    d_statistics.d_mbqi_inst_lemmas += mb->d_addedLemmas;
  }else{
    Trace("inst-fmf-ei") << "-> Exhaustive instantiate " << f << ", effort = " << effort << "..." << std::endl;
    Debug("inst-fmf-ei") << "   Instantiation Constants: ";
    for( size_t i=0; i<f[0].getNumChildren(); i++ ){
      Debug("inst-fmf-ei") << d_quantEngine->getTermDatabase()->getInstantiationConstant( f, i ) << " ";
    }
    Debug("inst-fmf-ei") << std::endl;
    //create a rep set iterator and iterate over the (relevant) domain of the quantifier
    RepSetIterator riter( d_quantEngine, &(d_quantEngine->getModel()->d_rep_set) );
    if( riter.setQuantifier( f ) ){
      Debug("inst-fmf-ei") << "Begin instantiation..." << std::endl;
      int triedLemmas = 0;
      int addedLemmas = 0;
      while( !riter.isFinished() && ( addedLemmas==0 || !options::fmfOneInstPerRound() ) ){
        //instantiation was not shown to be true, construct the match
        InstMatch m( f );
        for( int i=0; i<riter.getNumTerms(); i++ ){
          m.set( d_quantEngine, riter.d_index_order[i], riter.getTerm( i ) );
        }
        Debug("fmf-model-eval") << "* Add instantiation " << m << std::endl;
        triedLemmas++;
        //add as instantiation
        if( d_quantEngine->addInstantiation( f, m ) ){
          addedLemmas++;
        }else{
          Debug("fmf-model-eval") << "* Failed Add instantiation " << m << std::endl;
        }
        riter.increment();
      }
      d_addedLemmas += addedLemmas;
      d_triedLemmas += triedLemmas;
      d_statistics.d_exh_inst_lemmas += addedLemmas;
    }
    //if the iterator is incomplete, we will return unknown instead of sat if no instantiations are added this round
    d_incomplete_check = d_incomplete_check || riter.d_incomplete;
  }
}

void ModelEngine::debugPrint( const char* c ){
  Trace( c ) << "Quantifiers: " << std::endl;
  for( int i=0; i<(int)d_quantEngine->getModel()->getNumAssertedQuantifiers(); i++ ){
    Node f = d_quantEngine->getModel()->getAssertedQuantifier( i );
    Trace( c ) << "   ";
    if( !d_quantEngine->getModelBuilder()->isQuantifierActive( f ) ){
      Trace( c ) << "*Inactive* ";
    }else{
      Trace( c ) << "           ";
    }
    Trace( c ) << f << std::endl;
  }
  //d_quantEngine->getModel()->debugPrint( c );
}

ModelEngine::Statistics::Statistics():
  d_inst_rounds("ModelEngine::Inst_Rounds", 0),
  d_exh_inst_lemmas("ModelEngine::Instantiations_Exhaustive", 0 ),
  d_mbqi_inst_lemmas("ModelEngine::Instantiations_Mbqi", 0 )
{
  StatisticsRegistry::registerStat(&d_inst_rounds);
  StatisticsRegistry::registerStat(&d_exh_inst_lemmas);
  StatisticsRegistry::registerStat(&d_mbqi_inst_lemmas);
}

ModelEngine::Statistics::~Statistics(){
  StatisticsRegistry::unregisterStat(&d_inst_rounds);
  StatisticsRegistry::unregisterStat(&d_exh_inst_lemmas);
  StatisticsRegistry::unregisterStat(&d_mbqi_inst_lemmas);
}


