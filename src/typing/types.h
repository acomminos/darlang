#ifndef DARLANG_SRC_TYPING_TYPES_H_
#define DARLANG_SRC_TYPING_TYPES_H_

namespace darlang {
namespace typing {

class Function;
class Primitive;

// A concretely defined type, leveraging the visitor pattern to allow code
// generators (such as the LLVM backend) to produce appropriate IR.
//
// Types should be owned by a unscoped, global type registry.
class Type {
 public:
  struct Visitor {
    virtual void Type(Function& function_type) = 0;
    virtual void Type(Primitive& primitive_type) = 0;
  };

  virtual void Visit(Visitor& visitor) = 0;
};

// A function with zero or more arguments, returning a singular type.
class Function : public Type {
 public:
  Function(std::vector<Type*> arguments, Type* yields)
    : arguments_(arguments), yields_(yields) {}

  void Visit(Visitor& visitor) override { visitor.Type(*this); }

  std::vector<Type*> arguments() const { return arguments_; }
  const Type* yields() const { return yields_; }

 private:
  const std::vector<Type*> arguments_;
  const Type* yields_;
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

}  // namespace typing
}  // namespace darlang

#endif  // DARLANG_SRC_TYPING_TYPES_H_
