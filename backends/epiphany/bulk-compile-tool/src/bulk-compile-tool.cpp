// Inspired  from Eli Bendersky's blog post

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");

using std::vector;

class spmdcall {
  public:
    // Argument to the spmd call
    SourceRange originalArgument;
    // References from that argument
    vector<FunctionDecl*> references;
};

// Very ugly but to do this without globals is a lot of hassle:
// we'd have to manually create a FrontEndFactory and so on
std::vector<std::string> sourceList;

class ReferenceFinder : public RecursiveASTVisitor<ReferenceFinder> {
  public:
    ReferenceFinder(SourceManager& SM_, vector<FunctionDecl*>& reflist_)
        : SM(SM_), reflist(reflist_) {}

    // DeclRefExpr is anything that refers to another variable or function
    bool VisitDeclRefExpr(DeclRefExpr* e) {
        // See if it is a function
        if (FunctionDecl* f = dyn_cast_or_null<FunctionDecl>(e->getDecl())) {
            // See if we already had it (recursive functions)
            if (std::find(reflist.begin(), reflist.end(), f) != reflist.end())
                return true;
            // We do not have it yet
            // Check if it is in the main file
            auto range = f->getSourceRange();
            if (SM.isInMainFile(range.getBegin())) {
                reflist.push_back(f);
                // Traverse deeper
                TraverseDecl(f);
            }
        }
        // It could also be something else than a function
        // like a global or local variable but for now we do not
        // save those. Simply return.
        return true;
    }

  private:
    SourceManager& SM;
    std::vector<FunctionDecl*>& reflist;
};

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
  public:
    MyASTVisitor(SourceManager& SM_, vector<spmdcall>& calls_)
        : SM(SM_), calls(calls_) {}

    bool VisitCallExpr(CallExpr* c) {
        if (FunctionDecl* callee = c->getDirectCallee()) {
            if (callee->getQualifiedNameAsString() ==
                "bulk::environment<bulk::epiphany::provider>::spawn") {
                // Extract the first argument
                if (c->getNumArgs() == 2) {
                    Expr* arg = c->getArg(1);
                    if (arg->getType().getAsString() != "const char *") {
                        // If spawn is declared with std::function as argument
                        // it will be a CXXBindTemporaryExpr
                        // If it is declared with a normal function pointer it
                        // will
                        // probably be a ImplicitCastExpr

                        calls.push_back(spmdcall());
                        auto& call = calls.back();
                        call.originalArgument = arg->getSourceRange();
                        ReferenceFinder finder(SM, call.references);
                        finder.TraverseStmt(arg);
                    }
                }
            }
        }
        return true;
    }

  private:
    SourceManager& SM;
    vector<spmdcall>& calls;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
  public:
    MyASTConsumer(SourceManager& SM, vector<spmdcall>& vec)
        : Visitor(SM, vec) {}

    // Override the method that gets called for each parsed top-level
    // declaration.
    bool HandleTopLevelDecl(DeclGroupRef DR) override {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
            // Traverse the declaration using our AST visitor.
            Visitor.TraverseDecl(*b);
            //(*b)->dump();
        }
        return true;
    }

  private:
    MyASTVisitor Visitor;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
  public:
    MyFrontendAction() {}
    void EndSourceFileAction() override {

        std::stringstream hostfilename;
        hostfilename << filename << ".host.cpp";
        std::ofstream hostfile(hostfilename.str());
        if (!hostfile.is_open()) {
            llvm::errs() << "ERROR: Unable to open output file "
                         << hostfilename.str() << '\n';
        } else {
            for (size_t i = 0; i < spmdCalls.size(); ++i) {
                auto& call = spmdCalls[i];
                // Replace the original argument with a new one
                // First transform invalid characters
                std::stringstream kernel_cppname;
                std::stringstream kernel_elfname;
                std::string symbol_name;

                kernel_cppname << filename << '.' << i << ".kernel.cpp";
                kernel_elfname << filename << '.' << i << ".kernel.elf";

                // Symbolname is kernel name but with invalid characters
                // replaced by _
                symbol_name = kernel_elfname.str();
                std::transform(symbol_name.begin(), symbol_name.end(),
                               symbol_name.begin(), [](char c) {
                                   if ((c >= 'A' && c <= 'Z') ||
                                       (c >= 'a' && c <= 'z') ||
                                       (c >= '0' && c <= '9'))
                                       return c;
                                   else
                                       return '_';
                               });

                // Now emit the 'extern' things at top of host file
                hostfile << "extern \"C\" unsigned char _binary_" << symbol_name
                         << "_start;\n";
                hostfile << "extern \"C\" unsigned char _binary_" << symbol_name
                         << "_end;\n";

                // Replace the spawn arguments
                std::string originalArg =
                    TheRewriter.getRewrittenText(call.originalArgument);
                std::stringstream newarg;
                newarg << "std::make_pair(&_binary_" << symbol_name << "_start, &_binary_"
                       << symbol_name << "_end)";
                TheRewriter.ReplaceText(call.originalArgument, newarg.str());

                // Now write the kernel to a separate file
                std::ofstream file(kernel_cppname.str());
                if (!file.is_open()) {
                    llvm::errs() << "ERROR: Unable to open output file "
                                 << kernel_cppname.str() << '\n';
                    continue;
                }
                file << "#include <bulk/bulk.hpp>\n";
                file << "#include <bulk/backends/epiphany/epiphany.hpp>\n";
                file << "using bulk::epiphany::world;\n\n";
                for (int j = (int)call.references.size() - 1; j >= 0; j--) {
                    FunctionDecl* f = call.references[j];
                    auto functionText =
                        TheRewriter.getRewrittenText(f->getSourceRange());
                    file << "// Function '"
                         << f->getNameInfo().getName().getAsString()
                         << "' from main file referenced by kernel\n";
                    file << functionText << "\n\n";
                }
                file << "template<typename T> void run(T f) { f(world, "
                        "world.processor_id(), world.active_processors()); "
                        "}\n\n";
                file << "int main() {\n";
                file << "    run(" << originalArg << ");\n";
                file << "    world.sync();\n";
                file << "    return 0;\n}\n";
                file.close();
                // Output kernel filenames
                llvm::outs() << kernel_cppname.str() << '\n';
            }

            // Now emit the rewritten host buffer to a file
            {
                SourceManager& SM = TheRewriter.getSourceMgr();
                llvm::raw_os_ostream raw_file(hostfile);
                TheRewriter.getEditBuffer(SM.getMainFileID()).write(raw_file);
            }
            hostfile.close();
        }
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI,
                                                   StringRef file) override {
        SourceManager& SM = CI.getSourceManager();
        TheRewriter.setSourceMgr(SM, CI.getLangOpts());
        spmdCalls.clear();

        // file is absolute. However we want the relative path as was given on the commandline
        // therefore find it back in the command line list
        filename.clear();
        for (auto& s : sourceList) {
            if (getAbsolutePath(s) == file) {
                filename = s;
                break;
            }
        }
        if (filename.empty())
            filename = file;

        return llvm::make_unique<MyASTConsumer>(SM, spmdCalls);
    }

  private:
    Rewriter TheRewriter;
    std::vector<spmdcall> spmdCalls;
    std::string filename;
};

int main(int argc, const char** argv) {
    CommonOptionsParser op(argc, argv, ToolingSampleCategory);
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    sourceList = op.getSourcePathList();

    // ClangTool::run accepts a FrontendActionFactory, which is then used to
    // create new objects implementing the FrontendAction interface. Here we use
    // the helper newFrontendActionFactory to create a default factory that will
    // return a new MyFrontendAction object every time.
    // To further customize this, we could create our own factory class.
    return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
