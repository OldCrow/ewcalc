/// @file test_reference.cpp
/// @brief Tests for the static reference-data layer (#73).
///
/// UTF-8 source with non-ASCII literals; MSVC builds it with /utf-8
/// (set in ewpresenter/tests/CMakeLists.txt).

#include "ewpresenter/reference_data.h"
#include "test_main.h"

#include <cstring>
#include <string_view>

using namespace ewpresenter::refdata;

namespace {

bool eq(const char* a, std::string_view b) {
    return a != nullptr && std::string_view{a} == b;
}

void test_pages_shape() {
    const auto pages_span = pages();
    ASSERT_TRUE(pages_span.size() == 7);

    ASSERT_TRUE(eq(pages_span[0].id, "quick-values"));
    ASSERT_TRUE(eq(pages_span[0].title, "Quick Values"));
    ASSERT_TRUE(eq(pages_span[0].subtitle, "Common EW values for quick entry"));
    ASSERT_TRUE(pages_span[0].section_count == 5);
    ASSERT_TRUE(eq(pages_span[1].id, "ref-propagation"));
    ASSERT_TRUE(eq(pages_span[1].title, "Propagation"));
    ASSERT_TRUE(pages_span[1].section_count == 3);
    ASSERT_TRUE(eq(pages_span[2].id, "ref-antennas"));
    ASSERT_TRUE(eq(pages_span[2].title, "Antenna Types"));
    ASSERT_TRUE(pages_span[2].section_count == 11);
    ASSERT_TRUE(eq(pages_span[3].id, "ref-link"));
    ASSERT_TRUE(eq(pages_span[3].title, "Link Budget"));
    ASSERT_TRUE(pages_span[3].section_count == 2);
    ASSERT_TRUE(eq(pages_span[4].id, "ref-bands"));
    ASSERT_TRUE(eq(pages_span[4].title, "Frequency Bands"));
    ASSERT_TRUE(pages_span[4].section_count == 3);
    ASSERT_TRUE(eq(pages_span[5].id, "ref-glossary"));
    ASSERT_TRUE(eq(pages_span[5].title, "Glossary"));
    ASSERT_TRUE(pages_span[5].section_count == 7);
    ASSERT_TRUE(eq(pages_span[6].id, "ref-db-units"));
    ASSERT_TRUE(eq(pages_span[6].title, "dB & Units"));
    ASSERT_TRUE(pages_span[6].section_count == 2);
}

void test_antenna_types_page() {
    const Page& page = pages()[2];
    // Every antenna-type section carries its pattern thumbnail.
    for (std::size_t s = 0; s < page.section_count; ++s) {
        ASSERT_TRUE(page.sections[s].diagram != nullptr);
    }
    ASSERT_TRUE(eq(page.sections[0].title, "Isotropic Reference"));
    ASSERT_TRUE(page.sections[0].rows[0].kind == RowKind::Formula);
    ASSERT_TRUE(eq(page.sections[0].rows[0].svg_base, "aperture"));
    // The dipole gain copy value matches the Quick Values table's 2.15.
    ASSERT_TRUE(eq(page.sections[1].title, "Half-Wave Dipole"));
    ASSERT_TRUE(eq(page.sections[1].rows[1].copy_value, "2.15"));
    // Type sections carry the five spec rows.
    for (std::size_t s = 1; s < page.section_count; ++s) {
        ASSERT_TRUE(page.sections[s].row_count == 5);
    }
}

void test_link_page() {
    const Page& page = pages()[3];
    ASSERT_TRUE(eq(page.sections[0].title, "One-Way Link"));
    ASSERT_TRUE(eq(page.sections[0].diagram, "link-budget"));
    ASSERT_TRUE(page.sections[1].diagram == nullptr);
    ASSERT_TRUE(page.sections[0].row_count == 3);
    ASSERT_TRUE(page.sections[1].row_count == 3);
    // Margin and effective range are single-form (no log column/asset).
    ASSERT_TRUE(page.sections[0].rows[1].log_value == nullptr);
    ASSERT_TRUE(page.sections[0].rows[2].log_value == nullptr);
    // FSPL row reuses the shared asset pair and quotes the same constant.
    const Row& fspl = page.sections[1].rows[2];
    ASSERT_TRUE(eq(fspl.svg_base, "fspl"));
    ASSERT_TRUE(std::string_view{fspl.log_value}.find("32.44")
                != std::string_view::npos);
    // Sensitivity log form carries the -114 dBm/MHz anchor.
    ASSERT_TRUE(std::string_view{page.sections[1].rows[1].log_value}
                    .find("114") != std::string_view::npos);
}

void test_glossary_page() {
    const Page& page = pages()[5];
    // Pure value rows, no diagrams, no copy buttons — definitions only.
    for (std::size_t s = 0; s < page.section_count; ++s) {
        ASSERT_TRUE(page.sections[s].diagram == nullptr);
        for (std::size_t r = 0; r < page.sections[s].row_count; ++r) {
            const Row& row = page.sections[s].rows[r];
            ASSERT_TRUE(row.kind == RowKind::Value);
            ASSERT_TRUE(row.copy_value == nullptr);
        }
    }
    // The ERP/EIRP pair records the 2.15 dB offset and the convention note.
    ASSERT_TRUE(eq(page.sections[0].rows[0].label, "ERP"));
    ASSERT_TRUE(std::string_view{page.sections[0].rows[1].value}.find("2.15")
                != std::string_view::npos);
    ASSERT_TRUE(eq(page.sections[0].rows[2].label, "Convention here"));
}

void test_bands_page() {
    const Page& page = pages()[4];
    ASSERT_TRUE(page.sections[0].row_count == 13); // IEEE incl. mm(G)
    ASSERT_TRUE(page.sections[1].row_count == 13); // NATO A–M
    ASSERT_TRUE(page.sections[2].row_count == 8);  // ITU bands 4–11
    ASSERT_TRUE(eq(page.sections[0].rows[6].label, "X"));
    ASSERT_TRUE(eq(page.sections[0].rows[6].value, "8–12 GHz"));
    ASSERT_TRUE(eq(page.sections[1].rows[9].label, "J"));
    ASSERT_TRUE(eq(page.sections[1].rows[9].value, "10–20 GHz"));
}

void test_section_titles_and_counts() {
    const Page& page = pages()[0];
    ASSERT_TRUE(eq(page.sections[0].title, "Antenna Gain"));
    ASSERT_TRUE(page.sections[0].row_count == 8);
    ASSERT_TRUE(page.sections[1].row_count == 4);
    ASSERT_TRUE(eq(page.sections[2].title, "Thermal Noise Floor  (kT, 290 K)"));
    ASSERT_TRUE(page.sections[2].row_count == 6);
    ASSERT_TRUE(page.sections[3].row_count == 8);
    ASSERT_TRUE(page.sections[4].row_count == 5);
}

void test_propagation_page() {
    const Page& page = pages()[1];
    // Every section carries a #72 diagram thumbnail; quick-values has none.
    ASSERT_TRUE(eq(page.sections[0].title, "Path Loss"));
    ASSERT_TRUE(eq(page.sections[0].diagram, "prop-two-ray"));
    ASSERT_TRUE(page.sections[0].row_count == 3);
    ASSERT_TRUE(eq(page.sections[1].diagram, "prop-knife-edge"));
    ASSERT_TRUE(eq(page.sections[2].diagram, "prop-horizon-bulge"));
    ASSERT_TRUE(pages()[0].sections[0].diagram == nullptr);

    // All propagation rows are formulas with both forms and an asset base.
    for (std::size_t s = 0; s < page.section_count; ++s) {
        for (std::size_t r = 0; r < page.sections[s].row_count; ++r) {
            const Row& row = page.sections[s].rows[r];
            ASSERT_TRUE(row.kind == RowKind::Formula);
            ASSERT_TRUE(row.svg_base != nullptr);
            ASSERT_TRUE(row.log_value != nullptr);
        }
    }
    // Constants must match libew: FSPL 32.44, Fresnel /24 000, horizon 4.122.
    ASSERT_TRUE(std::string_view{page.sections[0].rows[0].log_value}
                    .find("32.44") != std::string_view::npos);
    ASSERT_TRUE(std::string_view{page.sections[0].rows[2].log_value}
                    .find("24 000") != std::string_view::npos);
    ASSERT_TRUE(std::string_view{page.sections[2].rows[1].log_value}
                    .find("4.122") != std::string_view::npos);
}

void test_db_units_page() {
    const Page& page = pages()[6];
    ASSERT_TRUE(eq(page.sections[0].title, "Ratio → dB"));
    ASSERT_TRUE(page.sections[0].row_count == 8);
    ASSERT_TRUE(page.sections[1].row_count == 6);
    // Conversion-constant rows carry copy values; prose rows do not.
    ASSERT_TRUE(eq(page.sections[1].rows[1].copy_value, "30"));   // dBW
    ASSERT_TRUE(eq(page.sections[1].rows[3].copy_value, "2.15")); // dBd
    ASSERT_TRUE(page.sections[1].rows[0].copy_value == nullptr);  // dBm
}

void test_value_row_fields() {
    const Section& gain = pages()[0].sections[0];
    const Row& iso = gain.rows[0];
    ASSERT_TRUE(iso.kind == RowKind::Value);
    ASSERT_TRUE(eq(iso.label, "Isotropic (reference)"));
    ASSERT_TRUE(eq(iso.value, "0.0 dBi"));
    ASSERT_TRUE(eq(iso.copy_value, "0.0"));
    ASSERT_TRUE(iso.log_value == nullptr);
    ASSERT_TRUE(iso.svg_base == nullptr);
}

void test_value_row_without_copy() {
    // "Typical back lobe" is a range — no copy button.
    const Section& sll = pages()[0].sections[1];
    const Row& back_lobe = sll.rows[3];
    ASSERT_TRUE(eq(back_lobe.label, "Typical back lobe"));
    ASSERT_TRUE(back_lobe.copy_value == nullptr);
}

void test_noise_floor_values_ascii_free() {
    // Spot-check a non-ASCII display string round-trips intact (the
    // /utf-8 guard this test exists for): U+2212 MINUS SIGN, not '-'.
    const Section& noise = pages()[0].sections[2];
    ASSERT_TRUE(eq(noise.rows[0].value, "−174.0 dBm"));
    ASSERT_TRUE(eq(noise.rows[0].copy_value, "-174.0"));
}

void test_formula_row_fields() {
    const Section& formulas = pages()[1].sections[0]; // Propagation / Path Loss
    const Row& fspl = formulas.rows[0];
    ASSERT_TRUE(fspl.kind == RowKind::Formula);
    ASSERT_TRUE(eq(fspl.label, "Free-space path loss"));
    ASSERT_TRUE(eq(fspl.svg_base, "fspl"));
    ASSERT_TRUE(fspl.value != nullptr);     // standard-form Unicode text
    ASSERT_TRUE(fspl.log_value != nullptr); // log-form Unicode text
    ASSERT_TRUE(fspl.copy_value == nullptr);
    // Log form must quote libew's FSPL constant.
    ASSERT_TRUE(std::string_view{fspl.log_value}.find("32.44")
                != std::string_view::npos);
}

void test_all_rows_have_label_and_value() {
    for (const Page& page : pages()) {
        for (std::size_t s = 0; s < page.section_count; ++s) {
            const Section& section = page.sections[s];
            ASSERT_TRUE(section.title != nullptr);
            for (std::size_t r = 0; r < section.row_count; ++r) {
                const Row& row = section.rows[r];
                if (row.label == nullptr || row.value == nullptr) {
                    ASSERT_TRUE(false);
                    return;
                }
                // Formula invariants: svg_base present; log asset iff text.
                if (row.kind == RowKind::Formula &&
                    (row.svg_base == nullptr || row.copy_value != nullptr)) {
                    ASSERT_TRUE(false);
                    return;
                }
            }
        }
    }
    ASSERT_TRUE(true);
}

} // namespace

TEST_MAIN()
    RUN_TEST(test_pages_shape);
    RUN_TEST(test_section_titles_and_counts);
    RUN_TEST(test_propagation_page);
    RUN_TEST(test_antenna_types_page);
    RUN_TEST(test_link_page);
    RUN_TEST(test_glossary_page);
    RUN_TEST(test_bands_page);
    RUN_TEST(test_db_units_page);
    RUN_TEST(test_value_row_fields);
    RUN_TEST(test_value_row_without_copy);
    RUN_TEST(test_noise_floor_values_ascii_free);
    RUN_TEST(test_formula_row_fields);
    RUN_TEST(test_all_rows_have_label_and_value);
    return test::summary();
}
