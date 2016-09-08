/*
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sstream>
#include <string>

#include "base/message_loop.h"
#include "cobalt/dom/document.h"
#include "cobalt/dom/document_type.h"
#include "cobalt/dom/element.h"
#include "cobalt/dom/html_element_context.h"
#include "cobalt/dom/serializer.h"
#include "cobalt/dom_parser/parser.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cobalt {
namespace dom {

class SerializerTest : public ::testing::Test {
 protected:
  SerializerTest();
  ~SerializerTest() OVERRIDE {}

  scoped_ptr<dom_parser::Parser> dom_parser_;
  scoped_ptr<DomStatTracker> dom_stat_tracker_;
  HTMLElementContext html_element_context_;
  scoped_refptr<Document> document_;
  scoped_refptr<Element> root_;
  base::SourceLocation source_location_;
  MessageLoop message_loop_;
};

SerializerTest::SerializerTest()
    : dom_parser_(new dom_parser::Parser()),
      dom_stat_tracker_(new DomStatTracker("SerializerTest")),
      html_element_context_(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, NULL, dom_stat_tracker_.get(), ""),
      document_(new Document(&html_element_context_)),
      root_(new Element(document_, base::Token("root"))),
      source_location_(base::SourceLocation("[object SerializerTest]", 1, 1)) {}

TEST_F(SerializerTest, Serialize) {
  const std::string input = "<div a><div b><div c></div></div></div>";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.Serialize(root_->first_element_child());
  EXPECT_EQ(input, oss.str());
}

TEST_F(SerializerTest, SerializeSelfOnly) {
  const std::string input = "<div a><div b><div c></div></div></div>";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.SerializeSelfOnly(root_->first_element_child());
  EXPECT_EQ("<div a>...</div>", oss.str());
}

TEST_F(SerializerTest, SerializeDescendantsOnly) {
  const std::string input = "<div a><div b><div c></div></div></div>";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.SerializeDescendantsOnly(root_->first_element_child());
  EXPECT_EQ("<div b><div c></div></div>", oss.str());
}

TEST_F(SerializerTest, Empty) {
  const std::string input = "";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.SerializeDescendantsOnly(root_);
  EXPECT_EQ(input, oss.str());
}

TEST_F(SerializerTest, Comment) {
  const std::string input = "<div><!-- some comment --></div>";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.SerializeDescendantsOnly(root_);
  EXPECT_EQ(input, oss.str());
}

TEST_F(SerializerTest, Document) {
  const std::string input = "<html><body><div></div></body></html>";
  document_ = dom_parser_->ParseDocument(input, &html_element_context_,
                                         source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.Serialize(document_);
  EXPECT_EQ(input, oss.str());
}

TEST_F(SerializerTest, DocumentType) {
  // Currently doctype is not generated by the parser, so not using it here.
  scoped_refptr<DocumentType> doctype =
      new DocumentType(document_, base::Token("html"), "", "");

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.Serialize(doctype);
  EXPECT_EQ("<!DOCTYPE html>", oss.str());
}

TEST_F(SerializerTest, ElementBothOpeningAndClosingTagShouldBeOutput) {
  const std::string input = "<br/>";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.SerializeDescendantsOnly(root_);
  EXPECT_EQ("<br></br>", oss.str());
}

TEST_F(SerializerTest, ElementAttributesShouldBeSorted) {
  const std::string input = "<div c b a></div>";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.SerializeDescendantsOnly(root_);
  EXPECT_EQ("<div a b c></div>", oss.str());
}

TEST_F(SerializerTest, ElementAttributeEmptyValuesShouldBeOmitted) {
  const std::string input = "<div a b=\"\" c=\"value\"></div>";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.SerializeDescendantsOnly(root_);
  EXPECT_EQ("<div a b c=\"value\"></div>", oss.str());
}

TEST_F(SerializerTest, ElementAttributeSpacesShouldBeCollapsed) {
  const std::string input = "<div a    b         c></div>";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.SerializeDescendantsOnly(root_);
  EXPECT_EQ("<div a b c></div>", oss.str());
}

TEST_F(SerializerTest, Text) {
  const std::string input = "<div> some text </div>";
  dom_parser_->ParseDocumentFragment(input, document_, root_, NULL,
                                     source_location_);

  std::ostringstream oss;
  Serializer serializer(&oss);
  serializer.SerializeDescendantsOnly(root_);
  EXPECT_EQ(input, oss.str());
}

}  // namespace dom
}  // namespace cobalt
