#pragma once
#include <QQuickImageProvider>
#include <Windows.h>
class ExeImageProvider : public QQuickImageProvider {
  public:
    ExeImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Image)
    {
    }

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override
    {
        HICON icon = 0;
        std::wstring path = id.toStdWString();
        icon = (HICON)LoadImage(
            0,
            path.data(),
            IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON),
            LR_LOADFROMFILE | LR_LOADMAP3DCOLORS);
        if (!icon) {
            ExtractIconEx(path.data(), 0, &icon, nullptr, 1);
            if (!icon) {
                return {};
            }
        } 
        return QImage::fromHICON(icon);
    }
};
