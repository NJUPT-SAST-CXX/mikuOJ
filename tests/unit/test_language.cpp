#include <gtest/gtest.h>

#include "cppjudge/language.h"

using namespace cppjudge;

TEST(LanguageManager, DetectFromExtension) {
    EXPECT_EQ(LanguageManager::detect_from_extension("solution.cpp"), Language::CPP);
    EXPECT_EQ(LanguageManager::detect_from_extension("main.cc"), Language::CPP);
    EXPECT_EQ(LanguageManager::detect_from_extension("src/main.cxx"), Language::CPP);
    EXPECT_EQ(LanguageManager::detect_from_extension("prog.c"), Language::C);
    EXPECT_EQ(LanguageManager::detect_from_extension("script.py"), Language::PYTHON3);
    EXPECT_EQ(LanguageManager::detect_from_extension("Main.java"), Language::JAVA);
    EXPECT_EQ(LanguageManager::detect_from_extension("main.go"), Language::GO);
    EXPECT_EQ(LanguageManager::detect_from_extension("main.rs"), Language::RUST);
}

TEST(LanguageManager, DetectUnknown) {
    EXPECT_EQ(LanguageManager::detect_from_extension("README.md"), Language::UNKNOWN);
    EXPECT_EQ(LanguageManager::detect_from_extension("Makefile"), Language::UNKNOWN);
}

TEST(LanguageManager, ParseStrings) {
    EXPECT_EQ(LanguageManager::parse("cpp"), Language::CPP);
    EXPECT_EQ(LanguageManager::parse("c++"), Language::CPP);
    EXPECT_EQ(LanguageManager::parse("python3"), Language::PYTHON3);
    EXPECT_EQ(LanguageManager::parse("python"), Language::PYTHON3);
    EXPECT_EQ(LanguageManager::parse("java"), Language::JAVA);
    EXPECT_EQ(LanguageManager::parse("haskell"), Language::UNKNOWN);
}

TEST(LanguageManager, SupportedAtLeastSix) {
    EXPECT_GE(LanguageManager::supported_languages().size(), 6u);
}

TEST(LanguageManager, RuntimeConfigConsistent) {
    for (auto lang : LanguageManager::supported_languages()) {
        const auto& rt = LanguageManager::get_runtime(lang);
        EXPECT_EQ(rt.lang, lang);
        EXPECT_FALSE(rt.name.empty());
        EXPECT_FALSE(rt.extensions.empty());
        EXPECT_FALSE(rt.source_name.empty());
    }
}

TEST(LanguageManager, JavaEntryClassIsMain) {
    const auto& rt = LanguageManager::get_runtime(Language::JAVA);
    EXPECT_EQ(rt.source_name, "Main.java");
    EXPECT_EQ(rt.artifact_name, "Main.class");
}

TEST(LanguageManager, InterpretedHasNoCompiler) {
    const auto& rt = LanguageManager::get_runtime(Language::PYTHON3);
    EXPECT_FALSE(rt.needs_compilation);
    EXPECT_TRUE(rt.compiler_path.empty());
}

TEST(LanguageTools, ResolveExisting) {
    EXPECT_FALSE(resolve_tool({"sh"}).empty());
    EXPECT_TRUE(resolve_tool({"definitely-not-a-real-tool-xyz123"}).empty());
}
