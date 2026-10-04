#include "kest_test.h"

KEST_TEST(kest_test_numeric_signed_and_unsigned_wire_words)
{
    uint32_t word;
    kest_numeric_format signed_q15 = {15, false, KEST_NUMERIC_SATURATE};
    kest_numeric_format unsigned_q16 = {16, true, KEST_NUMERIC_SATURATE};
    assert(kest_encode_numeric(0.25f, signed_q15, 16, &word) == NO_ERROR);
    assert(word == 0x2000);
    assert(kest_encode_numeric(-0.25f, signed_q15, 16, &word) == NO_ERROR);
    assert(word == 0xe000);
    assert(kest_encode_numeric(0.25f, unsigned_q16, 16, &word) == NO_ERROR);
    assert(word == 0x4000);
    assert(kest_encode_numeric(0.75f, unsigned_q16, 16, &word) == NO_ERROR);
    assert(word == 0xc000);
    assert(kest_encode_numeric(1.0f, unsigned_q16, 16, &word) == NO_ERROR);
    assert(word == 0xffff);
    assert(kest_encode_numeric(-0.25f, unsigned_q16, 16, &word) == NO_ERROR);
    assert(word == 0);
    assert(kest_encode_numeric(0.25f, (kest_numeric_format){24, true, 0}, 24, &word) == NO_ERROR);
    assert(word == 0x400000);
}

KEST_TEST(kest_test_numeric_rejection_preserves_destination)
{
    kest_numeric_format format = {16, true, KEST_NUMERIC_REJECT};
    uint32_t word = 0x12345678;
    const float invalid[] = {-0.25f, 1.0f, INFINITY, NAN};
    for (unsigned i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++)
    {
        assert(kest_encode_numeric(invalid[i], format, 16, &word) == ERR_VALUE_OUT_OF_BOUNDS);
        assert(word == 0x12345678);
    }
    float minimum, maximum;
    assert(kest_numeric_format_bounds(format, 16, &minimum, &maximum) == NO_ERROR);
    assert(minimum == 0 && maximum == 1.0f - ldexpf(1.0f, -16));
    assert(kest_encode_numeric(maximum, format, 16, &word) == NO_ERROR);
    assert(word == 0xffff);
    assert(kest_encode_numeric(0, (kest_numeric_format){17, true, 0}, 16, &word) == ERR_BAD_ARGS);
    assert(word == 0xffff);
}
