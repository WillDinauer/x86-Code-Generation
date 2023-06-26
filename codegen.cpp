//
// LLVM Codegen Template
//
// 21 May 2022  jpb  Creation.
// 24 May 2022  bpk  Change everything.

#include "x86.hpp"
#include <llvm/IR/BasicBlock.h>       // for BasicBlock
#include <llvm/IR/Function.h>         // for Function
#include <llvm/IR/Instructions.h>     // for the instruction enums
#include <llvm/IR/LLVMContext.h>      // for LLVMContext
#include <llvm/IR/Module.h>           // for Module
#include <llvm/IRReader/IRReader.h>   // for parseIRFile
#include <llvm/Support/SourceMgr.h>   // for SMDiagnostic
#include <llvm/Support/raw_ostream.h> // for errs, outs, raw_string_ostream
#include <memory>                     // for std::unique_ptr
#include <stack>                      // for std::stack

int main(int argc, char **argv) {
    if (argc < 2) {
        llvm::errs() << "Usage: " << argv[0] << " <IR file>\n";
        return 1;
    }

    // Parse the IR into a module.
    llvm::SMDiagnostic diag;
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module_ptr = llvm::parseIRFile(argv[1], diag, context);
    if (!module_ptr) {
        llvm::errs() << "Couldn't parse the IR!\n";
        return 1;
    }

    llvm::Module &module = *module_ptr;
    x86Program program(module);

    for (llvm::Function &function : module) {
        for (llvm::BasicBlock &block : function) {

            program.handle_block_begin(block);
            for (llvm::BasicBlock::const_iterator it = block.begin(); it != block.end(); it++) {
                llvm::Instruction const &instruction = llvm::cast<llvm::Instruction>(*it);

                llvm::errs() << "Got an instruction: ";
                instruction.print(llvm::errs());
                llvm::errs() << "\n";

                switch (instruction.getOpcode()) {
                case llvm::Instruction::Call:
                    program.handle_call(it);
                    break;
                case llvm::Instruction::Ret:
                    program.handle_ret(it);
                    break;
                case llvm::Instruction::Add:
                    program.handle_binop(it, "add");
                    break;
                case llvm::Instruction::Mul:
                    program.handle_binop(it, "mul");
                    break;
                case llvm::Instruction::Sub:
                    program.handle_binop(it, "sub");
                    break;
                case llvm::Instruction::SDiv:
                    // Note that idiv is a unary instruction in x86, so it can't be handled like add, sub or imul.
                    program.handle_binop(it, "div");
                    break;
                case llvm::Instruction::ICmp:
                    program.handle_icmp(it);
                    break;
                case llvm::Instruction::Br:
                    program.handle_br(it);
                    break;
                case llvm::Instruction::PHI:
                    // Phi nodes get handled by handle_block_begin
                    break;
                default:
                    llvm::errs() << "Can't deal with this instruction.\n";
                    break;
                }
                // llvm::errs() << "I'm gonna go dust out the slots now\n";
                program.dust_out_slots(it);
            }
        }
    }

    program.print(llvm::outs());

    return 0;
}
