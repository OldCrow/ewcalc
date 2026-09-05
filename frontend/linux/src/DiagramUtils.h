#pragma once
/// @file DiagramUtils.h
/// @brief Collapsible "Geometry" diagram group shared by the calculator pages
/// (issue #72).
///
/// The diagram PNGs are compiled in via resources/diagrams.qrc under the
/// ":/diagrams/" prefix. They are 2x renders (1280 px wide) with a
/// transparent background, legible on light and dark palettes; DiagramLabel
/// shows each at up to its natural 640 px logical width, shrinking with the
/// pane while preserving aspect ratio and never upscaling.

#include <QGroupBox>
#include <QLabel>
#include <QObject>
#include <QPixmap>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

/// A QLabel that scales its pixmap to the available width, preserving aspect
/// ratio and capping at the diagram's natural logical width (source pixel
/// width / 2, since the PNGs are 2x renders). setScaledContents() stays OFF;
/// scaling is done manually at the display device-pixel ratio so the image
/// stays crisp on hi-DPI screens.
class DiagramLabel : public QLabel
{
public:
    explicit DiagramLabel(const QString& resourcePath, QWidget* parent = nullptr)
        : QLabel(parent)
        , source_(resourcePath)
        , naturalWidth_(source_.isNull() ? 0 : source_.width() / 2)
    {
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setMinimumSize(1, 1);  // let the page shrink below the diagram size
        QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sp.setHeightForWidth(true);
        setSizePolicy(sp);
        // ":/diagrams/loc-aoa-cep.png" → "loc aoa cep"
        const QString base = resourcePath.section('/', -1).section('.', 0, 0);
        setAccessibleName(QString(base).replace('-', ' '));
    }

    bool hasHeightForWidth() const override { return naturalWidth_ > 0; }

    int heightForWidth(int w) const override
    {
        if (naturalWidth_ <= 0)
            return QLabel::heightForWidth(w);
        const int drawW = qMin(w, naturalWidth_);
        return drawW * source_.height() / source_.width();
    }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        rescale();
    }

private:
    void rescale()
    {
        if (naturalWidth_ <= 0)
            return;
        const int targetW = qMin(width(), naturalWidth_);
        if (targetW <= 0 || targetW == lastWidth_)
            return;
        lastWidth_ = targetW;
        const qreal dpr = devicePixelRatio();  // qreal in Qt6
        QPixmap scaled = source_.scaledToWidth(
            qRound(targetW * dpr), Qt::SmoothTransformation);
        scaled.setDevicePixelRatio(dpr);
        setPixmap(scaled);
    }

    QPixmap source_;
    int naturalWidth_ = 0;
    int lastWidth_    = -1;
};

/// Creates a collapsible "Geometry" QGroupBox showing the given diagram
/// resources (full ":/diagrams/..." paths), in order. The checkable group-box
/// title is the collapse toggle; the group starts collapsed to keep the input
/// groups compact, matching the collapsed-by-default DisclosureGroup on macOS.
inline QGroupBox* makeDiagramGroup(const QStringList& resourcePaths)
{
    auto* box = new QGroupBox(QStringLiteral("Geometry"));
    box->setCheckable(true);
    box->setChecked(false);
    box->setToolTip(QStringLiteral("Show or hide the geometry diagrams for this page"));
    box->setAccessibleName(QStringLiteral("Geometry diagrams"));

    // An unchecked QGroupBox only disables its children; wrap them in one
    // widget whose visibility tracks the check state so the group actually
    // collapses to its title bar.
    auto* content = new QWidget;
    auto* inner   = new QVBoxLayout(content);
    inner->setContentsMargins(0, 0, 0, 0);
    for (const QString& path : resourcePaths)
        inner->addWidget(new DiagramLabel(path));

    auto* outer = new QVBoxLayout(box);
    outer->addWidget(content);
    content->setVisible(false);

    QObject::connect(box, &QGroupBox::toggled, content, &QWidget::setVisible);
    return box;
}
