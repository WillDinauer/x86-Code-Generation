#pragma once

#include <llvm/IR/BasicBlock.h>       // for llvm::BasicBlock
#include <llvm/IR/Constants.h>        // for llvm::ConstantInt
#include <llvm/IR/Module.h>           // for llvm::Module
#include <llvm/Support/raw_ostream.h> // for llvm::raw_ostream
#include <map>                        // for std::map
#include <queue>                      // for std::priority_queue
#include <set>                        // for std::set
#include <utility>                    // for std::pair
#include <vector>                     // for std::vector

// Implements std::map::contains because babylon's gcc is too old
template <typename K, typename V> bool contains(std::map<K, V> const &map, K const &key);

template <typename K> bool contains(std::set<K> const &set, K const &key);

// Implements llvm::BasicBlock::isEntryBlock because babylon's llvm is too old.
bool is_entry_block(llvm::BasicBlock const &block);

// Convenience function. Returns whether @block begins with a phi node.
bool block_starts_with_phi(llvm::BasicBlock const &block);

// Abstract base class for a source operand to an instruction.
// Note that destinations can be sources, but not all sources can be destinations (eg. immediates)
// If I were doing this all over again, I would probably pass these around by value
// instead of constantly making heap allocations. However, it's kind of a pain to go change all the code.
struct x86Source {
    virtual void print(llvm::raw_ostream &) const = 0;
    virtual void print_as_pointer(llvm::raw_ostream &, int64_t) const = 0;
    virtual ~x86Source(void) = default;
    enum { IMM, REG, REG_PTR, IMM_PTR } type;
};

// Abstract base class for a destination operand to an instruction.
struct x86Destination : public x86Source {};

// An immediate operand to an instruction.
struct x86Immediate : public x86Source {
    int64_t val;

    x86Immediate(int64_t);
    x86Immediate(llvm::ConstantInt const &);
    void print(llvm::raw_ostream &) const;
    void print_as_pointer(llvm::raw_ostream &, int64_t _ = 0) const;
};

// A register operand to an instruction.
// Note that this class won't stop you from naming registers whatever you want. You shouldn't instantiate one of these
// with a name that doesn't exist.
struct x86Register : public x86Destination {
    std::string name;

    x86Register(std::string);
    void print(llvm::raw_ostream &) const;
    void print_as_pointer(llvm::raw_ostream &, int64_t offset = 0) const;
};

// A pointer operand to an instruction.
// Wraps either a register or an immediate.
// Note that this class won't stop you from making immediate-relative addresses,which don't exist in x86,
// so just don't do that.
struct x86Pointer : public x86Destination {
    x86Source *address;
    int64_t offset;

    x86Pointer(x86Source *, int64_t offset = 0);
    ~x86Pointer(void);
    void print(llvm::raw_ostream &) const;
    void print_as_pointer(llvm::raw_ostream &, int64_t) const;
};

// Abstract base class from which all instructions inherit
struct x86Instruction {
    virtual void print(llvm::raw_ostream &) const = 0;
    virtual ~x86Instruction(void) = default;
};

// Represents a label in x86 assembly.
// Labels aren't *really* instructions, but it's convenient to do subclass x86Instruction so they can sit alongside
// actual instructions in the x86Program instruction vector.
struct x86Label : public x86Instruction {
    std::string name;

    x86Label(std::string);
    std::string get_name(void) const;
    void print(llvm::raw_ostream &) const;
    void print_as_pointer(llvm::raw_ostream &) const;
};

// Represents a directive to the assembler, like `.globl`.
// Again, directives aren't actually instructions, but it's convenient.
struct x86Directive : public x86Instruction {
    std::string contents;

    x86Directive(std::string);
    void print(llvm::raw_ostream &) const;
};

// Represents a comment in x86 assembly.
// Again, not really an instruction, but convenient.
struct x86Comment : public x86Instruction {
    std::string contents;

    x86Comment(std::string);
    void print(llvm::raw_ostream &) const;
};

// Represents an instruction with no arguments, like `leave` or `ret`.
struct x86NoArgInstruction : public x86Instruction {
    std::string opcode;

    x86NoArgInstruction(std::string);
    void print(llvm::raw_ostream &) const;
};

// Represents an instruction with one source argument, like `push`.
struct x86SrcInstruction : public x86Instruction {
    std::string opcode;
    x86Source *source;

    x86SrcInstruction(std::string, x86Source *);
    ~x86SrcInstruction(void);
    void print(llvm::raw_ostream &) const;
};

// Represents an instruction with one destination argument, like `pop`.
struct x86DstInstruction : public x86Instruction {
    std::string opcode;
    x86Destination *destination;

    x86DstInstruction(std::string, x86Destination *);
    ~x86DstInstruction(void);
    void print(llvm::raw_ostream &) const;
};

// Represents an instruction with one immediate argument, like `int`.
struct x86ImmInstruction : public x86Instruction {
    std::string opcode;
    x86Immediate *immediate;

    x86ImmInstruction(std::string, x86Immediate *);
    ~x86ImmInstruction(void);
    void print(llvm::raw_ostream &) const;
};

// Represents an instruction with one label argument, like `call` or `jmp`.
// Note that this doesn't actually exist; labels are just immediate pointers, so there really are no x86 instructions
// that require labels. Still, it's easier to assume that `call` and `jmp` always go to labels than to allow arbitrary
// jumping.
struct x86LblInstruction : public x86Instruction {
    std::string opcode;
    x86Label *label;

    x86LblInstruction(std::string, x86Label *);
    // Note that we don't need a destructor because all labels will be deleted by the x86Program destructor
    void print(llvm::raw_ostream &) const;
};

// Represents an instruction with one source argument and one destination argument, like add or sub.
struct x86SrcDstInstruction : public x86Instruction {
    std::string opcode;
    x86Source *source;
    x86Destination *destination;

    x86SrcDstInstruction(std::string, x86Source *, x86Destination *);
    ~x86SrcDstInstruction(void);
    void print(llvm::raw_ostream &) const;
};

// The program. This is the main thing you need to fill out.
struct x86Program {
    // The sequence of instructions that makes up the program.
    std::vector<x86Instruction *> instructions;

    // Maps IR basic blocks to x86 labels.
    std::map<llvm::BasicBlock const *, x86Label *> labels;

    // Maps IR phi nodes to x86 labels.
    std::map<std::pair<llvm::BasicBlock const *, llvm::BasicBlock const *>, x86Label *> phi_node_labels;

    x86Program(llvm::Module const &);
    ~x86Program(void);
    void print(llvm::raw_ostream &) const;
    x86Destination *acquire_slot(llvm::Value const &);
    x86Destination *query_slot(llvm::Value const &);
    void release_slot(llvm::Value const &);
    void back_up_slots(x86Label *);
    void restore_slots(x86Label *);
    void insert_instruction(x86Instruction *);
    void handle_block_begin(llvm::BasicBlock const &);
    void dust_out_slots(llvm::BasicBlock::const_iterator);
    void handle_call(llvm::BasicBlock::const_iterator);
    void handle_ret(llvm::BasicBlock::const_iterator);
    void handle_br(llvm::BasicBlock::const_iterator);

    // Added as part of Project 3
    void handle_binop(llvm::BasicBlock::const_iterator, std::string op);
    void handle_icmp(llvm::BasicBlock::const_iterator);

    // I recommend that you not directly access the following data structures.
    // They are best accessed through the methods.

    // Note that %rbp and %rsp are callee-saved as well, but those get handled by the function prologue, leave, and ret.
    std::vector<std::string> const CALLEE_SAVED_REGISTERS{"rbx", "r12", "r13", "r14", "r15"};

    // Note that %rdi is caller-saved as well, but it's also used for argument passing so we won't deal with it here.
    std::vector<std::string> const CALLER_SAVED_REGISTERS{"rcx", "rdx", "rsi", "r8", "r9", "r10", "r11"};

    // This is the location of the top of the stack as an offset from %rbp.
    // Used for getting stack allocations
    int64_t top_of_stack = -8 * CALLEE_SAVED_REGISTERS.size();

    // A slot is just a destination with a priority, for internal use in the priority queue.
    typedef std::pair<int64_t, x86Destination *> slot;

    // These are all the register slots.
    // The notable omissions here are %rax, because it's for return values,
    //                                %rdi, because it's for arguments,
    //                                %rbp, because it's for the base pointer,
    //                                %rsp, because it's for the stack pointer
    std::map<std::string, uint64_t> const REGISTER_PRIORITIES{{"rbx", -12}, {"rcx", -11}, {"rdx", -10}, {"rsi", -9}, {"r8", -8},  {"r9", -7},
                                                              {"r10", -6},  {"r11", -5},  {"r12", -4},  {"r13", -3}, {"r14", -2}, {"r15", -1}};

    // This exists only for the priority queue.
    struct slot_comparator {
        bool operator()(slot const &s1, slot const &s2) {
            return s1.first > s2.first;
        }
    };

    // The available slots.
    std::priority_queue<slot, std::vector<slot>, slot_comparator> available_slots;

    // Map from each LLVM instruction with a result that currently occupies a slot to that slot.
    // The reason this can't just map to x86Destination * is that we need to reinsert slots from here into the queue.
    std::map<llvm::Value const *, slot> used_slots;

    // Backup copies of the state of the slots at the entry points to conditional branches. Used to restore the slots to
    // their previous states when entering the other side of a conditional branch.
    std::map<x86Label *, std::pair<std::priority_queue<slot, std::vector<slot>, slot_comparator>, std::map<llvm::Value const *, slot>>> slot_backups;
};
