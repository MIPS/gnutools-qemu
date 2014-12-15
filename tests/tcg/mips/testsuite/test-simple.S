#include "macros.inc"

TEST_ENTRY(simple)
        and    $t0, $0, $0
        ASSERT_ZERO($t0)
TEST_END
