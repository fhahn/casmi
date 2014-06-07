// gtest macros raise -Wsign-compare
#pragma GCC diagnostic ignored "-Wsign-compare"

#include "gtest/gtest.h"

#include "libsyntax/types.h"


TEST(TypeUnifyTest, unify_int_linked2_1) {
  Type t1(TypeType::UNKNOWN);
  Type t2(TypeType::UNKNOWN);

  t1.unify(&t2);

  Type res(TypeType::INT);

  EXPECT_TRUE(t1.unify(&res));
  EXPECT_TRUE(t1.is_complete());
  EXPECT_TRUE(t2.is_complete());
  EXPECT_TRUE(t1 == TypeType::INT);
  EXPECT_TRUE(t2 == TypeType::INT);
}

TEST(TypeUnifyTest, unify_int_linked2_2) {
  Type t1(TypeType::UNKNOWN);
  Type t2(TypeType::UNKNOWN);

  t1.unify(&t2);

  Type res(TypeType::INT);

  EXPECT_TRUE(t2.unify(&res));
  EXPECT_TRUE(t1.is_complete());
  EXPECT_TRUE(t2.is_complete());
  EXPECT_TRUE(t1 == TypeType::INT);
  EXPECT_TRUE(t2 == TypeType::INT);
}

TEST(TypeUnifyTest, unify_int_linked2_3) {
  Type t1(TypeType::UNKNOWN);
  Type t2(TypeType::UNKNOWN);

  t1.unify(&t2);

  Type res(TypeType::INT);

  EXPECT_TRUE(res.unify(&t1));
  EXPECT_TRUE(t1.is_complete());
  EXPECT_TRUE(t2.is_complete());
  EXPECT_TRUE(t1 == TypeType::INT);
  EXPECT_TRUE(t2 == TypeType::INT);
}

TEST(TypeUnifyTest, unify_int_linked2_4) {
  Type t1(TypeType::UNKNOWN);
  Type t2(TypeType::UNKNOWN);

  t1.unify(&t2);

  Type res(TypeType::INT);

  EXPECT_TRUE(res.unify(&t2));
  EXPECT_TRUE(t1.is_complete());
  EXPECT_TRUE(t2.is_complete());
  EXPECT_TRUE(t1 == TypeType::INT);
  EXPECT_TRUE(t2 == TypeType::INT);
}

TEST(TypeUnifyTest, unify_list_simple_1) {
  Type t1(TypeType::UNKNOWN);

  Type int_t(TypeType::INT);
  Type res(TypeType::LIST, &int_t);

  EXPECT_TRUE(res.unify(&t1));
  EXPECT_TRUE(t1.is_complete());
}

TEST(TypeUnifyTest, unify_list_simple_2) {
  Type t1(TypeType::UNKNOWN);

  Type int_t(TypeType::INT);
  Type res(TypeType::LIST, &int_t);

  EXPECT_TRUE(t1.unify(&res));
  EXPECT_TRUE(t1.is_complete());
  EXPECT_TRUE(t1 == TypeType::LIST);
  EXPECT_TRUE(*t1.subtypes[0] == TypeType::INT);
}

TEST(TypeUnifyTest, unify_list_internal_type_1) {

  Type t1(TypeType::UNKNOWN);
  Type list_t(TypeType::LIST, &t1);
  EXPECT_FALSE(list_t.is_complete());

  Type int_t(TypeType::INT);

  EXPECT_TRUE(t1.unify(&int_t));
  EXPECT_TRUE(t1.is_complete());
  EXPECT_TRUE(list_t.is_complete());
  EXPECT_TRUE(list_t == TypeType::LIST);
  EXPECT_TRUE(*list_t.subtypes[0] == TypeType::INT);
}

TEST(TypeUnifyTest, unify_list_list_1) {

  Type t1(TypeType::UNKNOWN);
  Type list_t(TypeType::LIST, &t1);

  Type int_t(TypeType::INT);
  Type target(TypeType::LIST, &int_t);

  EXPECT_TRUE(list_t.unify(&target));
  EXPECT_TRUE(t1.is_complete());
  EXPECT_TRUE(list_t.is_complete());
  EXPECT_TRUE(list_t == TypeType::LIST);
  EXPECT_TRUE(*list_t.subtypes[0] == TypeType::INT);
}

TEST(TypeUnifyTest, unify_list_list_2) {

  Type t1(TypeType::UNKNOWN);
  Type list_t(TypeType::LIST, &t1);

  Type int_t(TypeType::INT);
  Type target(TypeType::LIST, &int_t);

  EXPECT_TRUE(target.unify(&list_t));
  EXPECT_TRUE(t1.is_complete());
  EXPECT_TRUE(list_t.is_complete());
  EXPECT_TRUE(list_t == TypeType::LIST);
  EXPECT_TRUE(*list_t.subtypes[0] == TypeType::INT);
}

TEST(TypeUnifyTest, unify_simple_constraints_1) {
  Type int_t = Type(TypeType::INT);
  Type t1(TypeType::UNKNOWN);
  t1.constraints.push_back(&int_t);
  Type t2(TypeType::UNKNOWN);

  t1.unify(&t2);

  Type res(TypeType::FLOAT);

  EXPECT_FALSE(t1.unify(&res));
}

TEST(TypeUnifyTest, unify_simple_constraints_2) {
  Type int_t = Type(TypeType::INT);
  Type t1(TypeType::UNKNOWN);
  Type t2(TypeType::UNKNOWN);
  t2.constraints.push_back(&int_t);

  t1.unify(&t2);

  Type res(TypeType::FLOAT);

  EXPECT_FALSE(t1.unify(&res));
}

TEST(TypeUnifyTest, unify_simple_constraints_3) {
  Type int_t = Type(TypeType::INT);
  Type t1(TypeType::UNKNOWN);
  Type t2(TypeType::UNKNOWN);
  t2.constraints.push_back(&int_t);

  t1.unify(&t2);

  Type res(TypeType::FLOAT);

  EXPECT_FALSE(res.unify(&t1));
}
