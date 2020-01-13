#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
 
using namespace llvm;
 
namespace {
  
  struct DefUse : public FunctionPass {
    static char ID; 
    DefUse() : FunctionPass(ID) {}
 
    bool runOnFunction(Function &F) override {
 
    	errs() << "Function name: ";
    	errs() << F.getName() << '\n';
    	for(Function::iterator bb = F.begin(), e = F.end(); bb!=e; bb++)
    	{
    		for(BasicBlock::iterator i = bb->begin(), i2 = bb->end(); i!=i2; i++)
    		{
                Instruction * inst = dyn_cast<Instruction>(i);
                if(inst->getOpcode() == Instruction::Add || inst->getOpcode() == Instruction::ICmp)
                {
                    for(User *U: inst -> users())
                    {
                        if(Instruction * Inst = dyn_cast<Instruction>(U))
                        {
                            outs()<<"OpCode "<< inst->getOpcodeName() <<" used in :: ";
                            outs()<< * Inst <<"\n";
                        }
                    }
                }
    		}
    	}
 
    	return false;
    }
  };
}
 
char DefUse::ID = 0;
static RegisterPass<DefUse> X("DefUse", "This is def-use Pass");