#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
 
using namespace llvm;
 
namespace {
  
  struct UseDef : public FunctionPass {
    static char ID; 
    UseDef() : FunctionPass(ID) {}
 
    bool runOnFunction(Function &F) override {
 
    	errs() << "Function name: ";
    	errs() << F.getName() << '\n';
    	for(Function::iterator bb = F.begin(), e = F.end(); bb!=e; bb++)
    	{
    		for(BasicBlock::iterator i = bb->begin(), i2 = bb->end(); i!=i2; i++)
    		{
                Instruction * inst = dyn_cast<Instruction>(i);
                if(inst->getOpcode() == Instruction::Add)
                {
                    for(Use &U: inst -> operands())
                    {
                        Value * v = U.get();
                        outs()<< *v <<"\n";
                    }
                }
    		}
    	}
    	return false;
    }
  };
}
 
char UseDef::ID = 0;
static RegisterPass<UseDef> X("UseDef", "This is use-def Pass");