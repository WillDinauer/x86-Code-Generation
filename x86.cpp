#include "x86.hpp"
#include <llvm/IR/BasicBlock.h>       // for llvm::BasicBlock
#include <llvm/IR/Constants.h>        // for llvm::ConstantInt
#include <llvm/IR/Function.h>         // for llvm::Function
#include <llvm/IR/Instruction.h>      // for llvm::Instruction
#include <llvm/IR/Instructions.h>     // for CallInst
#include <llvm/IR/Module.h>           // for llvm::Module
#include <llvm/IR/Use.h>              // for llvm::Use
#include <llvm/IR/Value.h>            // for llvm::Value
#include <llvm/Support/Casting.h>     // for llvm::isa, llvm::cast
#include <llvm/Support/raw_ostream.h> // for llvm::raw_ostream, llvm::raw_string_ostream, llvm::errs, llvm::outs
#include <map>                        // for std::set
#include <set>                        // for std::map
#include <string>                     // for std::string

// Implements std::map::contains because babylon's gcc is too old
template <typename K, typename V> bool contains(std::map<K, V> const &map, K const &key) {
    return map.find(key) != map.end();
}

template <typename K> bool contains(std::set<K> const &set, K const &key) {
    return set.find(key) != set.end();
}

// Implements llvm::BasicBlock::isEntryBlock because babylon's llvm is too old.
bool is_entry_block(llvm::BasicBlock const &block) {
    return &block.getParent()->getEntryBlock() == &block;
}

// Convenience function. Returns whether @block begins with a phi node.
bool block_starts_with_phi(llvm::BasicBlock const &block) {
    return block.begin()->getOpcode() == llvm::Instruction::PHI;
}

// A set of all the slots. This exists so the destructors don't double-free slots used in more than one place.
// I really should be passing things by value. Oh well.
// global so it can be accessed from any destructor.
std::set<x86Source *> all_slots;

x86Immediate::x86Immediate(int64_t val) : val{val} {
    type = IMM;
}

x86Immediate::x86Immediate(llvm::ConstantInt const &constant_int) : val{constant_int.getSExtValue()} {
    type = IMM;
}

void x86Immediate::print(llvm::raw_ostream &os) const {
    os << "$" << val;
}

void x86Immediate::print_as_pointer(llvm::raw_ostream &os, int64_t _) const {
    os << val;
}

x86Register::x86Register(std::string name) : name{name} {
    type = REG;
}

void x86Register::print(llvm::raw_ostream &os) const {
    os << "%" << name;
}

void x86Register::print_as_pointer(llvm::raw_ostream &os, int64_t offset) const {
    os << offset << "(";
    print(os);
    os << ")";
}

x86Pointer::x86Pointer(x86Source *address, int64_t offset) : address{address}, offset{offset} {
    switch (address->type) {
    case IMM:
        type = IMM_PTR;
        break;
    case REG:
        type = REG_PTR;
        break;
    case REG_PTR:
        llvm::errs() << "ERROR: YOU'RE MAKING A POINTER TO A REG_PTR. THIS WILL NOT WORK.\n";
        break;
    case IMM_PTR:
        llvm::errs() << "ERROR: YOU'RE MAKING A POINTER TO AN IMM_PTR. THIS WILL NOT WORK.\n";
        break;
    }
}

x86Pointer::~x86Pointer(void) {
    if (!contains(all_slots, address)) {
        delete address;
    }
}

void x86Pointer::print(llvm::raw_ostream &os) const {
    address->print_as_pointer(os, offset);
}

void x86Pointer::print_as_pointer(llvm::raw_ostream &, int64_t) const {
    llvm::errs() << "ERROR: YOU CANNOT MAKE A POINTER OUT OF A POINTER. THIS WILL NOT ASSEMBLE.\n";
}

x86Label::x86Label(std::string name) : name{name} {
}

std::string x86Label::get_name(void) const {
    return name;
}

void x86Label::print(llvm::raw_ostream &os) const {
    os << name << ":\n";
}

x86Directive::x86Directive(std::string contents) : contents{contents} {
}

void x86Directive::print(llvm::raw_ostream &os) const {
    os << contents << "\n";
}

x86Comment::x86Comment(std::string contents) : contents{contents} {
}

void x86Comment::print(llvm::raw_ostream &os) const {
    os << "    # " << contents << "\n";
}

x86NoArgInstruction::x86NoArgInstruction(std::string opcode) : opcode{opcode} {
}

void x86NoArgInstruction::print(llvm::raw_ostream &os) const {
    os << "    " << opcode << "\n";
}

x86SrcInstruction::x86SrcInstruction(std::string opcode, x86Source *source) : opcode{opcode}, source{source} {
}

x86SrcInstruction::~x86SrcInstruction(void) {
    if (!contains(all_slots, source)) {
        delete source;
    }
}

void x86SrcInstruction::print(llvm::raw_ostream &os) const {
    os << "    " << opcode << " ";
    source->print(os);
    os << "\n";
}

x86DstInstruction::x86DstInstruction(std::string opcode, x86Destination *destination) : opcode{opcode}, destination{destination} {
}

x86DstInstruction::~x86DstInstruction(void) {
    if (!contains(all_slots, (x86Source *)destination)) {
        delete destination;
    }
}

void x86DstInstruction::print(llvm::raw_ostream &os) const {
    os << "    " << opcode << " ";
    destination->print(os);
    os << "\n";
}

x86ImmInstruction::x86ImmInstruction(std::string opcode, x86Immediate *immediate) : opcode{opcode}, immediate{immediate} {
}

x86ImmInstruction::~x86ImmInstruction(void) {
    delete immediate;
}

void x86ImmInstruction::print(llvm::raw_ostream &os) const {
    os << "    " << opcode << " ";
    immediate->print(os);
    os << "\n";
}

x86LblInstruction::x86LblInstruction(std::string opcode, x86Label *label) : opcode{opcode}, label{label} {
}

void x86LblInstruction::print(llvm::raw_ostream &os) const {
    os << "    " << opcode << " " << label->get_name() << "\n";
}

x86SrcDstInstruction::x86SrcDstInstruction(std::string opcode, x86Source *source, x86Destination *destination)
    : opcode{opcode}, source{source}, destination{destination} {
}

x86SrcDstInstruction::~x86SrcDstInstruction(void) {
    if (!contains(all_slots, source)) {
        delete source;
    }
    if (!contains(all_slots, (x86Source *)destination)) {
        delete destination;
    }
}

void x86SrcDstInstruction::print(llvm::raw_ostream &os) const {
    os << "    " << opcode << " ";
    source->print(os);
    os << ", ";
    destination->print(os);
    os << "\n";
}

// Constructs the program. Fills in the `labels` dictionary.
// Note: It's on you to put the labels in `instructions` in the appropriate places.
x86Program::x86Program(llvm::Module const &module) {
    x86Label *main_label = nullptr;

    // Make the basic block labels and stick them in the labels map.
    // Also fill in a map of instruction ids for later determining the latest use of each instruction.
    for (llvm::Function const &function : module) {
        for (llvm::BasicBlock const &block : function) {
            // The first block of a function should be labelled with the function's name.
            if (is_entry_block(block)) {
                std::string function_name(function.getName());
                x86Label *label = new x86Label(function_name);
                if (function_name == "main") {
                    main_label = label;
                }
                labels.insert({&block, label});
            }
            else {
                // Otherwise, just give it a unique name.
                std::string label;
                llvm::raw_string_ostream rsos(label);
                block.printAsOperand(rsos, false);
                label = rsos.str(); // This isn't necessary in llvm 13, but llvm 10 has a bug that requires this.

                label.replace(0, 1, "_block_");
                label = std::string("__") + std::string(function.getName()) + label;
                labels.insert({&block, new x86Label(label)});
            }

            std::set<llvm::BasicBlock const *> incoming_blocks_to_phi_batch;
            for (llvm::Instruction const &instruction : block) {
                if (!llvm::isa<llvm::PHINode>(instruction)) {
                    break;
                }
                llvm::PHINode const &phi_instruction = llvm::cast<llvm::PHINode>(instruction);

                for (llvm::BasicBlock const *incoming_block : phi_instruction.blocks()) {
                    incoming_blocks_to_phi_batch.insert(incoming_block);
                }
            }

            // Grab this block's name
            std::string block_label = labels[&block]->get_name();

            for (llvm::BasicBlock const *incoming_block : incoming_blocks_to_phi_batch) {
                // Make the incoming block's name
                // It might already exist, but who cares? It doesn't take long to make a string.

                std::string incoming_block_label;
                llvm::raw_string_ostream rsos(incoming_block_label);
                incoming_block->printAsOperand(rsos, false);
                incoming_block_label = rsos.str(); // This isn't necessary in llvm 13, but llvm 10 has a bug that requires this.

                incoming_block_label.replace(0, 1, "_block_");
                incoming_block_label = std::string("__") + std::string(incoming_block->getParent()->getName()) + incoming_block_label;

                phi_node_labels.insert(
                    {{incoming_block, &block}, new x86Label(std::string("__PHI_FROM_") + incoming_block_label + std::string("_TO_") + block_label)});
            }
        }
    }

    // Make the register slots
    for (auto const &[register_name, priority] : REGISTER_PRIORITIES) {
        x86Destination *d = new x86Register(register_name);
        available_slots.push({priority, d});
        all_slots.insert(d);
    }

    // Make sure there's a main
    if (main_label == nullptr) {
        llvm::errs() << "ERROR: THERE'S NO MAIN.\n";
    }

    // The program header
    insert_instruction(new x86Comment("this assembly generated by the cs257 code generator"));
    insert_instruction(new x86Directive(".globl _start"));
    insert_instruction(new x86Label("_start"));
    insert_instruction(new x86LblInstruction("callq", main_label));
    insert_instruction(new x86Comment("taking main's return value and putting it in %rbx to act as program exit code"));
    insert_instruction(new x86SrcDstInstruction("movq", new x86Register("rax"), new x86Register("rbx")));
    insert_instruction(new x86Comment("1 is the linux interrupt code for exit"));
    insert_instruction(new x86SrcDstInstruction("movq", new x86Immediate(1), new x86Register("rax")));
    insert_instruction(new x86Comment("passing control to the kernel"));
    insert_instruction(new x86ImmInstruction("int", new x86Immediate(0x80)));
}

x86Program::~x86Program(void) {
    for (x86Instruction *instruction : instructions) {
        delete instruction;
    }

    for (x86Source const *destination : all_slots) {
        delete destination;
    }

    // Will be a memory leak if this object doesn't have all of its labels in its instructions vector.
}

void x86Program::print(llvm::raw_ostream &os) const {
    for (x86Instruction *instruction : instructions) {
        instruction->print(os);
    }
}

x86Destination *x86Program::acquire_slot(llvm::Value const &instruction) {
    llvm::errs() << "Acquiring slot for ";
    instruction.print(llvm::errs());
    llvm::errs() << "\n";
    if (available_slots.empty()) {
        top_of_stack -= 8;
        slot s{-top_of_stack, new x86Pointer(new x86Register("rbp"), top_of_stack)};
        available_slots.push(s);
        all_slots.insert(s.second);
        insert_instruction(new x86SrcDstInstruction("sub", new x86Immediate(8), new x86Register("rsp")));
    }
    slot s = available_slots.top();
    available_slots.pop();
    used_slots.insert({&instruction, s});

    return s.second;
}

x86Destination *x86Program::query_slot(llvm::Value const &instruction) {
    return used_slots[&instruction].second;
}

void x86Program::release_slot(llvm::Value const &instruction) {
    slot s = used_slots[&instruction];
    used_slots.erase(&instruction);
    available_slots.push(s);
}

void x86Program::back_up_slots(x86Label *label) {
    slot_backups.insert({label, {available_slots, used_slots}});
}

void x86Program::restore_slots(x86Label *label) {
    auto &backup = slot_backups[label];
    available_slots = backup.first; // These are deliberately making copies
    used_slots = backup.second;     // These are deliberately making copies
    slot_backups.erase(label);
}

void x86Program::insert_instruction(x86Instruction *instruction) {
    instructions.push_back(instruction);
}

void x86Program::handle_block_begin(llvm::BasicBlock const &block) {
    // Insert the label for this block.
    // Even if there are phi nodes, I'm still leaving this here because it's easier.
    insert_instruction(labels[&block]);

    // If we have a slot backup for this block, restore from it.
    if (contains(slot_backups, labels[&block])) {
        llvm::errs() << "Restoring the slots.\n";
        restore_slots(labels[&block]);
    }

    if (is_entry_block(block)) {
        // Reset the stack.
        top_of_stack = -40;

        std::string function_name = labels[&block]->get_name();
        insert_instruction(new x86Comment("function prologue for " + function_name));
        insert_instruction(new x86SrcInstruction("pushq", new x86Register("rbp")));
        insert_instruction(new x86SrcDstInstruction("movq", new x86Register("rsp"), new x86Register("rbp")));

        insert_instruction(new x86Comment("pushing callee-saved registers for start of " + function_name));
        for (std::string register_name : CALLEE_SAVED_REGISTERS) {
            insert_instruction(new x86SrcInstruction("pushq", new x86Register(register_name)));
        }

        // Remember that all functions have at most 1 argument
        if (block.getParent()->arg_size() == 1) {
            llvm::Value const &arg = *block.getParent()->arg_begin();

            // Save the arg in a slot
            if (!arg.use_empty()) {
                insert_instruction(new x86Comment("saving the argument to " + function_name));
                insert_instruction(new x86SrcDstInstruction("movq", new x86Register("rdi"), acquire_slot(arg)));
            }
        }
    }

    if (block_starts_with_phi(block)) {
        // Make the set of incoming blocks and the set of phi nodes
        // Also acquire a slot for each phi node that has uses.
        std::set<llvm::BasicBlock const *> incoming_blocks_to_phi_batch; // set for deduplication
        std::vector<llvm::PHINode const *> phi_nodes;
        for (llvm::Instruction const &instruction : block) {
            if (!llvm::isa<llvm::PHINode>(instruction)) {
                break;
            }
            llvm::PHINode const &phi_instruction = llvm::cast<llvm::PHINode>(instruction);

            phi_nodes.push_back(&phi_instruction);

            // Note that we drop the slot on the ground here, but we'll pick it up with query_slot when we loop over the set.
            if (!phi_instruction.use_empty()) {
                acquire_slot(phi_instruction);
            }

            for (llvm::BasicBlock const *incoming_block : phi_instruction.blocks()) {
                incoming_blocks_to_phi_batch.insert(incoming_block);
            }
        }

        x86Label *phi_done = new x86Label(std::string("__PHI_DONE_") + labels[&block]->get_name());

        // Actually generate the code for the phi instructions.
        for (llvm::BasicBlock const *incoming_block : incoming_blocks_to_phi_batch) {
            if (contains(phi_node_labels, {incoming_block, &block})) {
                // The label for this phi edge:
                insert_instruction(phi_node_labels[{incoming_block, &block}]);

                // For each phi node,
                for (llvm::PHINode const *phi_node : phi_nodes) {
                    // if this block is actually a predecessor of the phi node,
                    if (phi_node->getBasicBlockIndex(incoming_block) != -1 && !phi_node->use_empty()) {
                        // grab the correct value for the phi node given the incoming block
                        llvm::Value const *incoming_value = phi_node->getIncomingValueForBlock(incoming_block);

                        x86Source *src = nullptr;
                        if (llvm::isa<llvm::ConstantInt>(incoming_value)) {
                            llvm::ConstantInt const &incoming_const_int = *llvm::cast<llvm::ConstantInt>(incoming_value);
                            src = new x86Immediate(incoming_const_int);
                        }
                        else {
                            src = query_slot(*incoming_value);
                        }

                        insert_instruction(new x86SrcDstInstruction("movq", src, query_slot(*phi_node)));
                    }
                }
                insert_instruction(new x86LblInstruction("jmp", phi_done));
            }
        }

        // Put in the phi_done label.
        insert_instruction(phi_done);
    }
}

// Helper method to x86Program::dust_out_slots
// Returns whether @instruction uses @value
bool instruction_makes_use_of(llvm::Instruction const &instruction, llvm::Value const *value) {
    // for (llvm::User const *user : value->users()) {
    //     if (&instruction == user) {
    //         return true;
    //     }
    // }

    for (llvm::Value const *operand : instruction.operands()) {
        if (operand == value) {
            return true;
        }
    }

    return false;
}

// Helper method to x86Program::dust_out_slots
// Returns whether @value has any uses reachable from just after @it (i.e. not including @it)
bool recursively_check_for_uses(llvm::BasicBlock const *block, llvm::Value const *value, std::set<llvm::BasicBlock const *> &seen) {
    if (value->isUsedInBasicBlock(block)) {
        return true;
    }

    llvm::Instruction const &terminator = *block->getTerminator();
    for (int i = 0; i < terminator.getNumSuccessors(); i++) {
        llvm::BasicBlock const *child_block = terminator.getSuccessor(i);
        if (!contains(seen, child_block)) {
            seen.insert(child_block);
            if (recursively_check_for_uses(child_block, value, seen)) {
                return true;
            }
        }
    }

    return false;
}

// Helper method to x86Program::dust_out_slots
// Returns whether @value has any uses reachable from just after @it (i.e. not including @it)
bool has_reachable_uses(llvm::BasicBlock::const_iterator it, llvm::Value const *value) {
    while (!it->isTerminator()) {
        it++;
        if (instruction_makes_use_of(*it, value)) {
            return true;
        }
    }

    // Note that `seen` starts out empty, so we may revisit the starting block if there's a loop.
    // This is actually good because if we can revisit the starting block, then there's definitely a reachable use.
    // (i.e. @it's instruction)
    std::set<llvm::BasicBlock const *> seen;

    // The terminator instruction of the block we started in
    llvm::Instruction const &terminator = llvm::cast<llvm::Instruction>(*it);
    for (int i = 0; i < terminator.getNumSuccessors(); i++) {
        llvm::BasicBlock const *child_block = terminator.getSuccessor(i);
        if (!contains(seen, child_block)) {
            seen.insert(child_block);
            if (recursively_check_for_uses(child_block, value, seen)) {
                return true;
            }
        }
    }

    return false;
}

// Try to clear out any slots made unnecessary after generating assembly from @it.
void x86Program::dust_out_slots(llvm::BasicBlock::const_iterator it) {
    std::vector<llvm::Value const *> values_to_try;
    for (auto const &[value, _] : used_slots) {
        values_to_try.push_back(value);
    }

    for (llvm::Value const *value : values_to_try) {
        // If this operand takes up a slot and has no reachable uses from here,
        if (contains(used_slots, value) && !has_reachable_uses(it, value)) {
            llvm::errs() << "Releasing the slot for ";
            value->print(llvm::errs());
            llvm::errs() << "\n";
            release_slot(*value);
        }
    }
}

void x86Program::handle_ret(llvm::BasicBlock::const_iterator it) {
    llvm::ReturnInst const &ret_instruction = llvm::cast<llvm::ReturnInst>(*it);
    llvm::Value const *return_value = ret_instruction.getReturnValue();
    if (return_value != nullptr) {
        insert_instruction(new x86Comment("sticking return value into %rax"));
        if (llvm::isa<llvm::ConstantInt>(*return_value)) {
            llvm::ConstantInt const &constant_int_return_value = llvm::cast<llvm::ConstantInt>(*return_value);
            insert_instruction(new x86SrcDstInstruction("movq", new x86Immediate(constant_int_return_value), new x86Register("rax")));
        }
        else {
            llvm::Value const &instruction_return_value = llvm::cast<llvm::Value>(*return_value);
            insert_instruction(new x86SrcDstInstruction("movq", query_slot(instruction_return_value), new x86Register("rax")));
        }
    }

    insert_instruction(new x86Comment("popping callee-saved registers"));
    int offset = -(8 * CALLEE_SAVED_REGISTERS.size());
    for (auto it = CALLEE_SAVED_REGISTERS.rbegin(); it != CALLEE_SAVED_REGISTERS.rend(); it++) {
        insert_instruction(new x86SrcDstInstruction("movq", new x86Pointer(new x86Register("rbp"), offset), new x86Register(*it)));
        offset += 8;
    }

    insert_instruction(new x86Comment("tearing down the stack and returning"));
    insert_instruction(new x86NoArgInstruction("leaveq"));
    insert_instruction(new x86NoArgInstruction("retq"));
}

void x86Program::handle_call(llvm::BasicBlock::const_iterator it) {
    llvm::CallInst const &call_instruction = llvm::cast<llvm::CallInst>(*it);

    llvm::BasicBlock const &entry_block = llvm::cast<llvm::Function>(*call_instruction.getCalledFunction()).getEntryBlock();

    std::string const &function_name = labels[&entry_block]->get_name();

    // Push the caller-saved registers
    insert_instruction(new x86Comment("pushing caller-saved registers before call to " + function_name));
    for (std::string register_name : CALLER_SAVED_REGISTERS) {
        insert_instruction(new x86SrcInstruction("pushq", new x86Register(register_name)));
    }

    // Pass the argument if there is one.
    // Remember that we are disallowing functions with more than one argument
    if (call_instruction.arg_size() != 0) {
        insert_instruction(new x86Comment("passing argument to " + function_name + " in %rdi"));
        llvm::Value const *arg = call_instruction.arg_begin()->get();
        x86Source *src = nullptr;
        if (llvm::isa<llvm::ConstantInt>(*arg)) {
            src = new x86Immediate(llvm::cast<llvm::ConstantInt>(*arg));
        }
        else {
            src = query_slot(*arg);
        }

        insert_instruction(new x86SrcDstInstruction("movq", src, new x86Register("rdi")));
    }

    insert_instruction(new x86Comment("calling " + function_name));
    insert_instruction(new x86LblInstruction("callq", labels[&entry_block]));

    // Pop the caller-saved registers
    insert_instruction(new x86Comment("popping caller-saved registers after call to " + function_name));
    for (auto it = CALLER_SAVED_REGISTERS.rbegin(); it != CALLER_SAVED_REGISTERS.rend(); it++) {
        insert_instruction(new x86DstInstruction("popq", new x86Register(*it)));
    }

    // At this point, the returned value (if there is one) is in %rax. If it needs to be saved, let's save it in a slot.
    if (!call_instruction.use_empty()) { // If the instruction has any uses
        insert_instruction(new x86Comment("saving the value returned from " + function_name));
        insert_instruction(new x86SrcDstInstruction("movq", new x86Register("rax"), acquire_slot(call_instruction)));
    }
}

void x86Program::handle_br(llvm::BasicBlock::const_iterator it) {
    llvm::BranchInst const &br_instruction = llvm::cast<llvm::BranchInst>(*it);

    // The block this br instruction is in
    llvm::BasicBlock const *this_block = it->getParent();

    // The block this br instruction goes to (the first one if there are 2)
    llvm::BasicBlock const *target_block_1 = br_instruction.getSuccessor(0);

    // Get the label for that block
    x86Label *target_label_1 = nullptr;
    if (block_starts_with_phi(*target_block_1)) {
        target_label_1 = phi_node_labels[{this_block, target_block_1}];
    }
    else {
        target_label_1 = labels[target_block_1];
    }

    // If the branch is unconditional, then we're done.
    if (br_instruction.isUnconditional()) {
        insert_instruction(new x86LblInstruction("jmp", target_label_1));
    }
    else if (br_instruction.isConditional()) {
        // The second block this br instruction goes to
        llvm::BasicBlock const *target_block_2 = br_instruction.getSuccessor(1);

        // Get the label for that block
        x86Label *target_label_2 = nullptr;
        if (block_starts_with_phi(*target_block_2)) {
            target_label_2 = phi_node_labels[{this_block, target_block_2}];
        }
        else {
            target_label_2 = labels[target_block_2];
        }

        // Figure out what types of jumps this br should create. (jl, jle, jg, jge, etc)
        llvm::Value const *cond = br_instruction.getCondition();
        if (llvm::isa<llvm::ICmpInst>(*cond)) {
            llvm::ICmpInst const &icmp = llvm::cast<llvm::ICmpInst>(*cond);

            // We're implementing llvm br with 2 x86 jumps, because if a jump's condition fails in x86, no jump occurs,
            // whereas in llvm a jump still occurs, but to the second branch.
            std::string opcode1("INVALID JUMP");
            std::string opcode2("INVALID JUMP");
            switch (icmp.getPredicate()) {
            case llvm::CmpInst::Predicate::ICMP_EQ:
                opcode1 = "je";
                opcode2 = "jne";
                break;
            case llvm::CmpInst::Predicate::ICMP_NE:
                opcode1 = "jne";
                opcode2 = "je";
                break;
            case llvm::CmpInst::Predicate::ICMP_SGT:
                opcode1 = "jg";
                opcode2 = "jle";
                break;
            case llvm::CmpInst::Predicate::ICMP_SGE:
                opcode1 = "jge";
                opcode2 = "jl";
                break;
            case llvm::CmpInst::Predicate::ICMP_SLT:
                opcode1 = "jl";
                opcode2 = "jge";
                break;
            case llvm::CmpInst::Predicate::ICMP_SLE:
                opcode1 = "jle";
                opcode2 = "jg";
                break;
            default:
                llvm::errs() << "ERROR: INVALID COMPARISON PREDICATE.\n";
                break;
            }

            insert_instruction(new x86LblInstruction(opcode1, target_label_1));
            insert_instruction(new x86LblInstruction(opcode2, target_label_2));

            llvm::errs() << "Backing up the slots.\n";
            back_up_slots(labels[br_instruction.getSuccessor(0)]);
            back_up_slots(labels[br_instruction.getSuccessor(1)]);
        }
        else {
            // If there's a constant in a branch condition, the dead code elimination pass should have taken care of it.
            llvm::errs() << "ERROR: INVALID TYPE OF BRANCH CONDITION.\n";
        }
    }
}

// Handles binary operators (add, sub, mul, div) in the LLVM pass, converting them to x86 assembly
void x86Program::handle_binop(llvm::BasicBlock::const_iterator it, std::string op) {
    llvm::BinaryOperator const &bop_inst = llvm::cast<llvm::BinaryOperator>(*it);

    llvm::Value *lhs = bop_inst.getOperand(0);          // get the left operand of the binary operation
    llvm::Value *rhs = bop_inst.getOperand(1);          // get the right operand of the binary operation

    insert_instruction(new x86Comment("Processing a binary operation"));
    
    // The sources for the instructions; either set to an x86Immediate or an x86Register depending on the left/right operands
    x86Source *l_src = nullptr;
    x86Source *r_src = nullptr;
    
    if (llvm::isa<llvm::ConstantInt>(lhs)) {
        // the left operand (lhs) is a constant. Thus, the left source is an x86Immediate with a value equal to the constant.
        l_src = new x86Immediate(llvm::cast<llvm::ConstantInt>(*lhs));
    } else {
        // the left operand (lhs) is not constant. Thus, the left source is an x86Register from a previous instruction
        l_src = query_slot(*lhs);
    }
    insert_instruction(new x86SrcDstInstruction("movq", l_src, new x86Register("rax")));        // move the left source into %rax

    if (llvm::isa<llvm::ConstantInt>(rhs)) {
        // rhs is constant, the right source is an x86Immediate with a value equal to rhs
        r_src = new x86Immediate(llvm::cast<llvm::ConstantInt>(*rhs));
    } else {
        // rhs is not a constant, the right source is an x86Register saved from a previous instruction
        r_src = query_slot(*rhs);
    }

    // if adding or subtracting, add an x86 'add' or 'sub' command with the right source as the source and %rax as the destination
    if (op.compare("add") == 0 || op.compare("sub") == 0) {
        insert_instruction(new x86SrcDstInstruction(op, r_src, new x86Register("rax")));
        // if multiplying or dividing, add an x86 'mul' or 'div' command with the right source as the source. Note that
        // 'mul' and 'div' implicitly apply their operations to %rax
    } else if (op.compare("mul") == 0 || op.compare("div") == 0) {
        insert_instruction(new x86SrcInstruction(op, r_src));
    } else {
      llvm::errs() << "ERROR: INVALID OPCODE.\n";
    }

    // Only save the result of the operation (currently stored in %rax) to a register if there are future uses of the instruction.
    // Otherwise, we would be wasting memory
    if (!bop_inst.use_empty()) {
        insert_instruction(new x86SrcDstInstruction("movq", new x86Register("rax"), acquire_slot(bop_inst)));
    }
    insert_instruction(new x86Comment("Finished processing binary operation"));
}

// Handles icmp instructions found during the LLVM pass, converting them to x86 assembly
void x86Program::handle_icmp(llvm::BasicBlock::const_iterator it) {
    llvm::CmpInst const &comp_inst = llvm::cast<llvm::CmpInst>(*it);    // cast the iterator to a CmpInst

    // Aquire the left and right operands
    llvm::Value *lhs = comp_inst.getOperand(0);
    llvm::Value *rhs = comp_inst.getOperand(1);

    insert_instruction(new x86Comment("Processing a comparison instruction"));

    // Set up the left and right source, which will ultimately either be an x86Immediate or an x86Register
    x86Source *l_src = nullptr;
    x86Source *r_src = nullptr;

    if (llvm::isa<llvm::ConstantInt>(lhs)) {
        // the left operand is a constant, so the left source is an x86Immediate with the value of the left operand.
        l_src = new x86Immediate(llvm::cast<llvm::ConstantInt>(*lhs));
    } else {
        // the left operand comes from a previous instruction, so grab the x86Register where the result of that previous operation is stored.
        l_src = query_slot(*lhs);
    }
    // move the left source (either an x86Immediate or an x86Register) into %rax
    insert_instruction(new x86SrcDstInstruction("movq", l_src, new x86Register("rax")));

    if (llvm::isa<llvm::ConstantInt>(rhs)) {
        // the right operand is a constant, so the right source is an x86Immediate with the value of the right operand.
        r_src = new x86Immediate(llvm::cast<llvm::ConstantInt>(*rhs));
    } else {
        // the rigth operand comes from a previous instruction, so grab the x86Register where the resultof that previous operation is stored.
        r_src = query_slot(*rhs);
    }

    // apply the 'cmp' instruction from the right source to %rax, which will properly set the flags for any upcoming jumps.
    insert_instruction(new x86SrcDstInstruction("cmp", r_src, new x86Register("rax")));
    insert_instruction(new x86Comment("Finished processing a comparison instruction"));
}
