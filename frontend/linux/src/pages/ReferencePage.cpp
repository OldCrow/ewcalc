// ReferencePage.cpp
#include "ReferencePage.h"

#include <initializer_list>

#include <QClipboard>
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

// ── Reference data ────────────────────────────────────────────────────────────

namespace {

struct RefEntry {
    const char* label;
    const char* value;
    const char* copyValue; ///< nullptr = no copy button
};

struct RefSection {
    const char* title;
    std::initializer_list<RefEntry> entries;
};

static const RefSection kSections[] = {
    {
        "Antenna Gain",
        {
            { "Isotropic (reference)",                "0.0 dBi",             "0.0"   },
            { "Short whip / rubber duck",             "0 dBi  (typical)",    "0"     },
            { "Quarter-wave whip, ground-plane mount","2 dBi  (typical)",    "2"     },
            { "Half-wave dipole",                     "2.15 dBi  (0 dBd)",   "2.15"  },
            { "2-element Yagi",                       "\u2248 7 dBi",         "7"     },
            { "3-element Yagi",                       "\u2248 8.5 dBi",       "8.5"   },
            { "5-element Yagi",                       "\u2248 10.5 dBi",      "10.5"  },
            { "10-element Yagi",                      "\u2248 14 dBi",        "14"    },
        }
    },
    {
        "Antenna Sidelobe Levels (re main lobe)",
        {
            { "Uniform aperture \u2014 1st SLL",  "\u221213 dBc",            "-13"   },
            { "Taylor weighted \u2014 1st SLL",   "\u221225 dBc",            "-25"   },
            { "Low-sidelobe array \u2014 1st SLL","\u221235 dBc",            "-35"   },
            { "Typical back lobe",                "\u221225 to \u221235 dBc", nullptr },
        }
    },
    {
        "Thermal Noise Floor  (kT, 290 K)",
        {
            { "1 Hz bandwidth",    "\u2212174.0 dBm", "-174.0" },
            { "1 kHz bandwidth",   "\u2212144.0 dBm", "-144.0" },
            { "1 MHz bandwidth",   "\u2212114.0 dBm", "-114.0" },
            { "10 MHz bandwidth",  "\u2212104.0 dBm", "-104.0" },
            { "100 MHz bandwidth", "\u221294.0 dBm",  "-94.0"  },
            { "1 GHz bandwidth",   "\u221284.0 dBm",  "-84.0"  },
        }
    },
    {
        "Radar Cross Section (Typical)",
        {
            { "Large aircraft (broadside)",        "+15 dBsm  (rep.)",  "15"  },
            { "Fighter (broadside)",               "+7 dBsm  (rep.)",   "7"   },
            { "Fighter (nose-on, conventional)",   "0 dBsm  (\u2248 1 m\u00b2)", "0" },
            { "LO fighter (nose-on)",              "\u221215 dBsm  (rep.)", "-15" },
            { "Cruise missile",                    "\u22127 dBsm  (rep.)",  "-7"  },
            { "Bird",                              "\u221215 dBsm  (rep.)", "-15" },
            { "Ship (small, \u223c1\u202f000 t)",  "+25 dBsm  (rep.)",  "25"  },
            { "Ship (large, >10\u202f000 t)",      "+45 dBsm  (rep.)",  "45"  },
        }
    },
    {
        "Eb/N\u2080 Requirements (AWGN)",
        {
            { "BPSK / QPSK,  BER 10\u207b\u00b3",      "6.8 dB",          "6.8"  },
            { "BPSK / QPSK,  BER 10\u207b\u2075",      "9.6 dB",          "9.6"  },
            { "BPSK / QPSK,  BER 10\u207b\u2076",      "10.5 dB",         "10.5" },
            { "Non-coh. FSK, BER 10\u207b\u00b3",      "\u2248 13.5 dB",   "13.5" },
            { "Non-coh. FSK, BER 10\u207b\u2075",      "\u2248 17.0 dB",   "17.0" },
        }
    },
};

/// Adds one reference row to @p form.
/// Rows with a copy value get a small "⧉" button; others get a spacer.
static void addRefRow(QFormLayout* form,
                      const QString& label,
                      const QString& value,
                      const QString& copyValue)
{
    auto* valLbl = new QLabel(value);
    valLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // All rows use the same HBox layout so value text aligns consistently:
    // [value label — stretch] [copy button or fixed spacer — 26px]
    auto* cell = new QWidget;
    auto* hbox = new QHBoxLayout(cell);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(4);
    hbox->addWidget(valLbl, 1);

    if (!copyValue.isEmpty()) {
        auto* btn = new QPushButton(QStringLiteral("\u29c9"));
        btn->setFixedWidth(26);
        btn->setFlat(true);
        btn->setToolTip(QStringLiteral("Copy ") + copyValue);
        const QString cv = copyValue;
        QObject::connect(btn, &QPushButton::clicked, btn, [cv](){
            QGuiApplication::clipboard()->setText(cv);
        });
        hbox->addWidget(btn, 0);
    } else {
        // Fixed-width spacer keeps value text at the same right edge
        // as rows that have a copy button.
        auto* spacer = new QWidget;
        spacer->setFixedWidth(26);
        hbox->addWidget(spacer, 0);
    }
    form->addRow(label + ':', cell);
}

} // namespace

// ── ReferencePage ─────────────────────────────────────────────────────────────

ReferencePage::ReferencePage(QWidget* parent)
    : QWidget(parent)
{
    auto* content = new QWidget;
    auto* vbox    = new QVBoxLayout(content);

    for (const auto& section : kSections) {
        QFormLayout* form = nullptr;
        auto* box  = new QGroupBox(QString::fromUtf8(section.title));
        form = new QFormLayout(box);

        for (const auto& e : section.entries) {
            addRefRow(form,
                      QString::fromUtf8(e.label),
                      QString::fromUtf8(e.value),
                      e.copyValue ? QString::fromUtf8(e.copyValue) : QString{});
        }
        vbox->addWidget(box);
    }
    vbox->addStretch();

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}
