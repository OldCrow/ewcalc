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
    ASSERT_TRUE(pages_span.size() == 13);

    // Quick Values retired 2026-09-12: its tables were redistributed to
    // the Antenna Types (gains, sidelobes) and Receiver (noise floor)
    // pages. Order: general references, then calculator order, with RCS
    // preceding Radar & Detection.
    ASSERT_TRUE(eq(pages_span[0].id, "ref-glossary"));
    ASSERT_TRUE(pages_span[0].section_count == 7);
    ASSERT_TRUE(eq(pages_span[1].id, "ref-db-units"));
    ASSERT_TRUE(pages_span[1].section_count == 2);
    ASSERT_TRUE(eq(pages_span[2].id, "ref-bands"));
    ASSERT_TRUE(pages_span[2].section_count == 3);
    ASSERT_TRUE(eq(pages_span[3].id, "ref-propagation"));
    ASSERT_TRUE(pages_span[3].section_count == 3);
    ASSERT_TRUE(eq(pages_span[4].id, "ref-antennas"));
    ASSERT_TRUE(pages_span[4].section_count == 12);
    ASSERT_TRUE(eq(pages_span[5].id, "ref-link"));
    ASSERT_TRUE(pages_span[5].section_count == 2);
    ASSERT_TRUE(eq(pages_span[6].id, "ref-receiver"));
    ASSERT_TRUE(pages_span[6].section_count == 4);
    ASSERT_TRUE(eq(pages_span[7].id, "ref-jamming"));
    ASSERT_TRUE(pages_span[7].section_count == 5);
    ASSERT_TRUE(eq(pages_span[8].id, "ref-location"));
    ASSERT_TRUE(pages_span[8].section_count == 3);
    ASSERT_TRUE(eq(pages_span[9].id, "ref-rcs"));
    ASSERT_TRUE(pages_span[9].section_count == 2);
    ASSERT_TRUE(eq(pages_span[10].id, "ref-radar-det"));
    ASSERT_TRUE(pages_span[10].section_count == 3);
    ASSERT_TRUE(eq(pages_span[11].id, "ref-doppler"));
    ASSERT_TRUE(pages_span[11].section_count == 2);
    ASSERT_TRUE(eq(pages_span[12].id, "ref-digital"));
    ASSERT_TRUE(pages_span[12].section_count == 3);
}

void test_antenna_types_page() {
    const Page& page = pages()[4];
    // Every antenna-type section carries its pattern thumbnail; the
    // trailing Sidelobe Levels table (ex-Quick Values) has none.
    for (std::size_t s = 0; s < page.section_count - 1; ++s) {
        ASSERT_TRUE(page.sections[s].diagram != nullptr);
    }
    ASSERT_TRUE(page.sections[page.section_count - 1].diagram == nullptr);
    ASSERT_TRUE(eq(page.sections[0].title, "Isotropic Reference"));
    ASSERT_TRUE(page.sections[0].rows[0].kind == RowKind::Formula);
    ASSERT_TRUE(eq(page.sections[0].rows[0].svg_base, "aperture"));
    // The dipole gain copy value matches the Quick Values table's 2.15.
    ASSERT_TRUE(eq(page.sections[1].title, "Half-Wave Dipole"));
    ASSERT_TRUE(eq(page.sections[1].rows[1].copy_value, "2.15"));
    // Monopole and Yagi absorbed the ex-Quick-Values gain ladders.
    ASSERT_TRUE(page.sections[2].row_count == 7);  // monopole + 2
    ASSERT_TRUE(page.sections[6].row_count == 9);  // yagi + 4
    ASSERT_TRUE(eq(page.sections[6].rows[5].copy_value, "14")); // 10-element
}

void test_receiver_page() {
    const Page& page = pages()[6];
    // The sensitivity row shares the Link Budget page's asset pair and text.
    const Row& sens = page.sections[0].rows[0];
    ASSERT_TRUE(eq(sens.svg_base, "sensitivity"));
    ASSERT_TRUE(std::string_view{sens.log_value}.find("114")
                != std::string_view::npos);
    // The noise-floor ladder (ex-Quick Values) sits beside sensitivity.
    ASSERT_TRUE(eq(page.sections[1].title, "Thermal Noise Floor  (kT, 290 K)"));
    ASSERT_TRUE(page.sections[1].row_count == 6);
    ASSERT_TRUE(eq(page.sections[1].rows[2].copy_value, "-114.0"));
    // Cascade section carries the #72 diagram; its formula is single-form.
    ASSERT_TRUE(eq(page.sections[2].diagram, "receiver-cascade"));
    ASSERT_TRUE(page.sections[2].rows[0].log_value == nullptr);
    // DR and SQNR are distinct rows (resolved fidelity docket).
    ASSERT_TRUE(eq(page.sections[3].rows[0].svg_base, "ddr"));
    ASSERT_TRUE(eq(page.sections[3].rows[1].svg_base, "sqnr"));
    ASSERT_TRUE(std::string_view{page.sections[3].rows[1].value}.find("1.76")
                != std::string_view::npos);
}

void test_jamming_page() {
    const Page& page = pages()[7];
    ASSERT_TRUE(page.section_count == 5);
    // The #72 radar-jamming geometry diagrams pair with the RADAR J/S
    // sections (user correction 2026-09-12); comms has its own section.
    ASSERT_TRUE(eq(page.sections[0].diagram, "jamming-self-protection"));
    ASSERT_TRUE(eq(page.sections[1].diagram, "jamming-stand-off"));
    ASSERT_TRUE(page.sections[2].diagram == nullptr);
    // Radar forms carry the 71 dB constant and the R4-vs-R2 contrast note.
    ASSERT_TRUE(std::string_view{page.sections[0].rows[0].value}.find("71")
                != std::string_view::npos);
    ASSERT_TRUE(std::string_view{page.sections[1].rows[0].value}
                    .find("40 log") != std::string_view::npos);
    ASSERT_TRUE(eq(page.sections[2].rows[0].svg_base, "js"));
    // All jamming formulas are single-form; burnthrough quotes 32.44.
    for (std::size_t s2 = 0; s2 < page.section_count; ++s2)
        for (std::size_t r = 0; r < page.sections[s2].row_count; ++r)
            ASSERT_TRUE(page.sections[s2].rows[r].log_value == nullptr);
    ASSERT_TRUE(std::string_view{page.sections[3].rows[0].value}.find("32.44")
                != std::string_view::npos);
}

void test_location_page() {
    const Page& page = pages()[8];
    ASSERT_TRUE(eq(page.sections[0].diagram, "loc-aoa-cep"));
    ASSERT_TRUE(eq(page.sections[2].diagram, "loc-eep-cep"));
    // Both Wegner EEP->CEP forms are shown, ours first (Eq 24a with 0.59).
    ASSERT_TRUE(std::string_view{page.sections[2].rows[0].value}.find("0.59")
                != std::string_view::npos);
    ASSERT_TRUE(std::string_view{page.sections[2].rows[1].value}.find("0.75")
                != std::string_view::npos);
    ASSERT_TRUE(page.sections[2].rows[1].kind == RowKind::Formula);
}

void test_radar_det_page() {
    const Page& page = pages()[10];
    // Range equation is the page's only std+log pair; its log form is the
    // corrected 40·log10 arrangement (the /4 doc line was a typo).
    const Row& rng = page.sections[0].rows[0];
    ASSERT_TRUE(eq(rng.svg_base, "radarrange"));
    ASSERT_TRUE(std::string_view{rng.log_value}.find("40 log")
                != std::string_view::npos);
    // Albersheim/Shnidman are method anchors (value rows), not formulas.
    ASSERT_TRUE(page.sections[1].rows[0].kind == RowKind::Value);
    ASSERT_TRUE(page.sections[1].rows[1].kind == RowKind::Value);
    // Swerling table covers cases 0-4 plus the rule of thumb.
    ASSERT_TRUE(page.sections[2].row_count == 6);
}

void test_doppler_page() {
    const Page& page = pages()[11];
    ASSERT_TRUE(eq(page.sections[1].diagram, "resolution-cell"));
    // The dilemma product row is present (test_radar.cpp guards the math).
    bool found_dilemma = false;
    for (std::size_t r = 0; r < page.sections[0].row_count; ++r) {
        if (page.sections[0].rows[r].svg_base != nullptr &&
            std::string_view{page.sections[0].rows[r].svg_base} == "dilemma") {
            found_dilemma = true;
        }
    }
    ASSERT_TRUE(found_dilemma);
    // Doppler and range resolution are the page's two std+log pairs.
    ASSERT_TRUE(page.sections[0].rows[0].log_value != nullptr);
    ASSERT_TRUE(page.sections[1].rows[0].log_value != nullptr);
}

void test_digital_page() {
    const Page& page = pages()[12];
    // The Eb/N0 pair carries forward and inverse forms.
    ASSERT_TRUE(eq(page.sections[0].rows[0].svg_base, "ebno"));
    ASSERT_TRUE(page.sections[0].rows[0].log_value != nullptr);
    // The modulation table supersedes QV's (five audited rows + Shannon).
    ASSERT_TRUE(page.sections[1].row_count == 6);
    ASSERT_TRUE(eq(page.sections[1].rows[1].copy_value, "9.6"));
    ASSERT_TRUE(eq(page.sections[1].rows[5].copy_value, "-1.6"));
}

void test_rcs_page() {
    const Page& page = pages()[9];
    ASSERT_TRUE(eq(page.sections[0].title, "Simple Shapes"));
    ASSERT_TRUE(eq(page.sections[0].diagram, "rcs-regions"));
    // Six single-form shape formulas, then two value notes.
    for (std::size_t r = 0; r < 6; ++r) {
        ASSERT_TRUE(page.sections[0].rows[r].kind == RowKind::Formula);
        ASSERT_TRUE(page.sections[0].rows[r].log_value == nullptr);
    }
    ASSERT_TRUE(page.sections[0].rows[6].kind == RowKind::Value);
    // Targets carry both units; copy value is the dBsm number.
    ASSERT_TRUE(page.sections[1].row_count == 11);
    const Row& person = page.sections[1].rows[2];
    ASSERT_TRUE(eq(person.label, "Person"));
    ASSERT_TRUE(std::string_view{person.value}.find("m²")
                != std::string_view::npos);
    ASSERT_TRUE(eq(person.copy_value, "0"));
}

void test_link_page() {
    const Page& page = pages()[5];
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
    const Page& page = pages()[0];
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
    const Page& page = pages()[2];
    ASSERT_TRUE(page.sections[0].row_count == 13); // IEEE incl. mm(G)
    ASSERT_TRUE(page.sections[1].row_count == 13); // NATO A–M
    ASSERT_TRUE(page.sections[2].row_count == 8);  // ITU bands 4–11
    ASSERT_TRUE(eq(page.sections[0].rows[6].label, "X"));
    ASSERT_TRUE(eq(page.sections[0].rows[6].value, "8–12 GHz"));
    ASSERT_TRUE(eq(page.sections[1].rows[9].label, "J"));
    ASSERT_TRUE(eq(page.sections[1].rows[9].value, "10–20 GHz"));
}

void test_propagation_page() {
    const Page& page = pages()[3];
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
    const Page& page = pages()[1];
    ASSERT_TRUE(eq(page.sections[0].title, "Ratio → dB"));
    ASSERT_TRUE(page.sections[0].row_count == 8);
    ASSERT_TRUE(page.sections[1].row_count == 6);
    // Conversion-constant rows carry copy values; prose rows do not.
    ASSERT_TRUE(eq(page.sections[1].rows[1].copy_value, "30"));   // dBW
    ASSERT_TRUE(eq(page.sections[1].rows[3].copy_value, "2.15")); // dBd
    ASSERT_TRUE(page.sections[1].rows[0].copy_value == nullptr);  // dBm
}

void test_value_row_fields() {
    // Directivity row on the Antenna Types isotropic section.
    const Section& iso_sec = pages()[4].sections[0];
    const Row& iso = iso_sec.rows[1];
    ASSERT_TRUE(iso.kind == RowKind::Value);
    ASSERT_TRUE(eq(iso.label, "Directivity"));
    ASSERT_TRUE(eq(iso.copy_value, "0"));
    ASSERT_TRUE(iso.log_value == nullptr);
    ASSERT_TRUE(iso.svg_base == nullptr);
}

void test_value_row_without_copy() {
    // "Typical back lobe" is a range — no copy button (now on the
    // Antenna Types page's Sidelobe Levels section).
    const Section& sll = pages()[4].sections[11];
    const Row& back_lobe = sll.rows[3];
    ASSERT_TRUE(eq(back_lobe.label, "Typical back lobe"));
    ASSERT_TRUE(back_lobe.copy_value == nullptr);
}

void test_noise_floor_values_ascii_free() {
    // Spot-check a non-ASCII display string round-trips intact (the
    // /utf-8 guard this test exists for): U+2212 MINUS SIGN, not '-'.
    // The ladder now lives on the Receiver page.
    const Section& noise = pages()[6].sections[1];
    ASSERT_TRUE(eq(noise.rows[0].value, "−174.0 dBm"));
    ASSERT_TRUE(eq(noise.rows[0].copy_value, "-174.0"));
}

void test_formula_row_fields() {
    const Section& formulas = pages()[3].sections[0]; // Propagation / Path Loss
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
    RUN_TEST(test_propagation_page);
    RUN_TEST(test_antenna_types_page);
    RUN_TEST(test_link_page);
    RUN_TEST(test_receiver_page);
    RUN_TEST(test_jamming_page);
    RUN_TEST(test_location_page);
    RUN_TEST(test_radar_det_page);
    RUN_TEST(test_doppler_page);
    RUN_TEST(test_digital_page);
    RUN_TEST(test_rcs_page);
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
