#ifndef DARLANG_SRC_TYPING_TYPES_H_
#define DARLANG_SRC_TYPING_TYPES_H_

#include <memory>
#include <vector>

namespace darlang {
namespace typing {

class Function;
class Tuple;
class Primitive;
class DisjointUnion;
class Recurrence;

// A concretely defined type, leveraging the visitor pattern to allow code
// generators (such as the LLVM backend) to produce appropriate IR.
//
// Types should be owned by a unscoped, global type registry.
class Type {
 public:
  struct Visitor {
    virtual void Type(Function& function_type) = 0;
    virtual void Type(Tuple& tuple_type) = 0;
    virtual void Type(Primitive& primitive_type) = 0;
    virtual void Type(DisjointUnion& disjoint_type) = 0;
    virtual void Type(Recurrence& recurrence) = 0;
  };

  virtual void Visit(Visitor& visitor) = 0;
};

// A function with zero or more arguments, returning a singular type.
class Function : public Type {
 public:
  Function(std::vector<std::unique_ptr<Type>> arguments, std::unique_ptr<Type> yields)
    : arguments_(std::move(arguments)), yields_(std::move(yields)) {}

  void Visit(Visitor& visitor) override { visitor.Type(*this); }

  const std::vector<std::unique_ptr<Type>>& arguments() const { return arguments_; }
  const std::unique_ptr<Type>& yields() const { return yields_; }

 private:
  const std::vector<std::unique_ptr<Type>> arguments_;
  const std::unique_ptr<Type> yields_;
};

// An ordered sequence of data.
class Tuple : public Type {
 public:
  typedef std::tuple<std::string, std::unique_ptr<Type>> TaggedType;

  Tuple(std::vector<TaggedType> types) : types_(std::move(types)) {}

  void Visit(Visitor& visitor) override { visitor.Type(*this); }

  const std::vector<TaggedType>& types() const { return types_; }

 private:
  const std::vector<TaggedType> types_;
};

// TODO(acomminos): make enum members consistently cased
enum class PrimitiveType {
  Int64,
  Float,
  Boolean,
  String,
};

class Primitive : public Type {
 public:
  Primitive(PrimitiveType type) : type_(type) {}

  void Visit(Visitor& visitor) override { visitor.Type(*this); }

  PrimitiveType type() const { return type_; }

 private:
  const PrimitiveType type_;
};

class DisjointUnion : public Type {
 public:
  DisjointUnion(std::vector<std::unique_ptr<Type>> types)
    : types_(std::move(types)) {}

  void Visit(Visitor& visitor) override { visitor.Type(*this); }

  const std::vector<std::unique_ptr<Type>>& types() const {
    return types_;
  }

 private:
  const std::vector<std::unique_ptr<Type>> types_;
};


// A self-referential component of a type. Intended to be modeled using a
// pointer to a parent type. Required to unify variable-length structures (e.g.
// linked lists).
class Recurrence : public Type {
 public:
  // A stub constructor, to be used as a placeholder before the parent type is
  // fully synthesized.
  Recurrence() : parent_type_(nullptr) {}

  void Visit(Visitor& visitor) override { visitor.Type(*this); }

  void set_parent_type(const Type* type) { parent_type_ = type; }
  const Type* parent_type() const { return parent_type_; }

 private:
  const Type* parent_type_;
};

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPES_H_
