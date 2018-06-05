#include "typing/module_specializer.h"
#include "typing/primitive_solver.h"
#include "typing/intrinsics.h"

namespace darlang::typing {

ModuleSpecializer::ModuleSpecializer(Logger& log, bool is_program)
  : log_(log), is_program_(is_program) {}

SpecializationMap& ModuleSpecializer::Specialize(ast::Node& node) {
  node.Visit(*this);
  return specs_;
}

bool ModuleSpecializer::Module(ast::ModuleNode& node) {
  // TODO(acomminos): add support for exports

  util::DeclarationMap decl_map = util::DeclarationMapper::Map(node);
  Specializer specializer(log_, decl_map);

  // XXX(acomminos): add skeleton typeables for ALL intrinsics
  LoadIntrinsic(Intrinsic::IS, specializer);
  LoadIntrinsic(Intrinsic::MOD, specializer);

  if (is_program_) {
    // TODO(acomminos): have main take in command-line args
    Result res;
    TypeablePtr main_return_type;
    if (!(res = specializer.Specialize("main", {}, main_return_type))) {
      log_.Fatal(res, node.start);
    }

    // Ensure that the specialized main function returns an integer.
    auto return_solver = std::make_unique<PrimitiveSolver>(PrimitiveType::Int64);
    auto return_type = Typeable::Create(std::move(return_solver));
    if (!(res = return_type->Unify(main_return_type))) {
      log_.Fatal(res, node.start);
    }
  }

  specs_ = specializer.specs();

  return false;
}

}  // namespace darlang::typing
