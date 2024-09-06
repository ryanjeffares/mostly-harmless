//
// Created by Syl Morrison on 11/08/2024.
//
#include <choc/gui/choc_DesktopWindow.h>
#include <choc/gui/choc_WebView.h>
#include <mostly_harmless/gui/mostlyharmless_WebviewEditor.h>
#if defined(MOSTLY_HARMLESS_MACOS)
#include <mostly_harmless/gui/platform/mostlyharmless_GuiHelpersMacOS.h>
#elif defined(MOSTLY_HARMLESS_WINDOWS)
#include <windef.h>
#include <winuser.h>
#endif
#include <cassert>
#include <filesystem>
namespace mostly_harmless::gui {
    static std::unordered_map<std::string, std::string> s_mimeTypes = {
        { "aac", "audio/aac" },
        { "aif", "audio/aiff" },
        { "aiff", "audio/aiff" },
        { "avif", "image/avif" },
        { "bmp", "image/bmp" },
        { "css", "text/css" },
        { "csv", "text/csv" },
        { "flac", "audio/flac" },
        { "gif", "image/gif" },
        { "htm", "text/html" },
        { "html", "text/html" },
        { "ico", "image/vnd.microsoft.icon" },
        { "jpeg", "image/jpeg" },
        { "jpg", "image/jpeg" },
        { "js", "text/javascript" },
        { "json", "application/json" },
        { "md", "text/markdown" },
        { "mid", "audio/midi" },
        { "midi", "audio/midi" },
        { "mjs", "text/javascript" },
        { "mp3", "audio/mpeg" },
        { "mp4", "video/mp4" },
        { "mpeg", "video/mpeg" },
        { "ogg", "audio/ogg" },
        { "otf", "font/otf" },
        { "pdf", "application/pdf" },
        { "png", "image/png" },
        { "rtf", "application/rtf" },
        { "svg", "image/svg+xml" },
        { "svgz", "image/svg+xml" },
        { "tif", "image/tiff" },
        { "tiff", "image/tiff" },
        { "ttf", "font/ttf" },
        { "txt", "text/plain" },
        { "wasm", "application/wasm" },
        { "wav", "audio/wav" },
        { "weba", "audio/webm" },
        { "webm", "video/webm" },
        { "webp", "image/webp" },
        { "woff", "font/woff" },
        { "woff2", "font/woff2" },
        { "xml", "application/xml" },
        { "zip", "application/zip" },
    };

    std::optional<std::string> getMimeType(const std::string& filename) {
        const auto ext = std::filesystem::path{ filename }.extension().string().substr(1);
        auto it = s_mimeTypes.find(ext);
        if (it == s_mimeTypes.end()) return {};
        return s_mimeTypes[ext];
    }

#if defined(MOSTLY_HARMLESS_WINDOWS)
    class WebviewEditor::Impl {
    public:
        Impl(std::uint32_t initialWidth, std::uint32_t initialHeight) : m_initialWidth(initialWidth), m_initialHeight(initialHeight) {
        }

        void setOptions(choc::ui::WebView::Options&& opts) {
            m_options = std::move(opts);
        }

        void create() {
            const auto iWidth = static_cast<int>(m_initialWidth);
            const auto iHeight = static_cast<int>(m_initialHeight);
            ::SetEnvironmentVariable("WEBVIEW2_DEFAULT_BACKGROUND_COLOR", "0");
            m_webview = std::make_unique<choc::ui::WebView>(m_options);
            auto* hwnd = static_cast<::HWND>(m_webview->getViewHandle());
            ::SetWindowPos(hwnd, NULL, 0, 0, iWidth, iHeight, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
        }

        void destroy() {
            m_webview.reset();
        }

        void getSize(std::uint32_t* width, std::uint32_t* height) {
            auto* handle = static_cast<HWND>(m_webview->getViewHandle());
            ::RECT rect;
            ::GetClientRect(handle, &rect);
            *width = static_cast<std::uint32_t>(rect.right - rect.left);
            *height = static_cast<std::uint32_t>(rect.bottom - rect.top);
        }

        void setSize(std::uint32_t width, std::uint32_t height) {
            auto* hwnd = static_cast<::HWND>(m_webview->getViewHandle());
            ::SetWindowPos(hwnd, NULL, 0, 0, static_cast<int>(width), static_cast<int>(height), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
        }

        void setParent(void* parentHandle) {
            auto* parentHwnd = static_cast<HWND>(parentHandle);
            auto* handle = static_cast<::HWND>(m_webview->getViewHandle());
            ::SetWindowLongPtrW(handle, GWL_STYLE, WS_CHILD);
            ::SetParent(handle, parentHwnd);
            show();
        }

        void show() {
            auto* webviewHwnd = static_cast<HWND>(m_webview->getViewHandle());
            ::ShowWindow(webviewHwnd, SW_SHOW);
        }

        void hide() {
            auto* webviewHwnd = static_cast<HWND>(m_webview->getViewHandle());
            ::ShowWindow(webviewHwnd, SW_HIDE);
        }

        [[nodiscard]] choc::ui::WebView* getWebview() noexcept {
            return m_webview.get();
        }

    private:
        std::uint32_t m_initialWidth{ 0 }, m_initialHeight{ 0 };
        choc::ui::WebView::Options m_options;
        std::unique_ptr<choc::ui::WebView> m_webview{ nullptr };
    };

#elif defined(MOSTLY_HARMLESS_MACOS)
    class WebviewEditor::Impl {
    public:
        Impl(std::uint32_t initialWidth, std::uint32_t initialHeight) : m_initialWidth(initialWidth),
                                                                        m_initialHeight(initialHeight) {
        }

        void setOptions(choc::ui::WebView::Options&& opts) {
            m_options = std::move(opts);
        }

        void create() {
            m_webview = std::make_unique<choc::ui::WebView>(m_options);
            helpers::macos::setViewSize(m_webview->getViewHandle(), m_initialWidth, m_initialHeight);
        }

        void destroy() {
            //            helpers::macos::removeFromParentView(m_webview->getViewHandle());
            m_webview.reset();
        }

        void getSize(std::uint32_t* width, std::uint32_t* height) {
            helpers::macos::getViewSize(m_webview->getViewHandle(), width, height);
        }

        void setSize(std::uint32_t width, std::uint32_t height) {
            helpers::macos::setViewSize(m_webview->getViewHandle(), width, height);
        }

        void setParent(void* parentHandle) {
            helpers::macos::reparentView(parentHandle, m_webview->getViewHandle());
        }

        void show() {
            helpers::macos::showView(m_webview->getViewHandle());
        }

        void hide() {
            helpers::macos::hideView(m_webview->getViewHandle());
        }

        [[nodiscard]] choc::ui::WebView* getWebview() noexcept {
            return m_webview.get();
        }

    private:
        std::uint32_t m_initialWidth{ 0 }, m_initialHeight{ 0 };
        choc::ui::WebView::Options m_options;
        std::unique_ptr<choc::ui::WebView> m_webview{ nullptr };
    };
#else
    static_assert(false);
#endif

    WebviewEditor::WebviewEditor(std::uint32_t initialWidth, std::uint32_t initialHeight) {
        m_impl = std::make_unique<WebviewEditor::Impl>(initialWidth, initialHeight);
    }
    WebviewEditor::~WebviewEditor() noexcept {
    }

    void WebviewEditor::setOptions(choc::ui::WebView::Options&& opts) noexcept {
        m_impl->setOptions(std::move(opts));
    }

    void WebviewEditor::initialise(EditorContext /*context*/) {
        m_impl->create();
        m_internalWebview = m_impl->getWebview();
    }

    void WebviewEditor::destroy() {
        m_internalWebview = nullptr;
        m_impl->destroy();
    }

    void WebviewEditor::getSize(std::uint32_t* width, std::uint32_t* height) {
        m_impl->getSize(width, height);
    }

    void WebviewEditor::setSize(std::uint32_t width, std::uint32_t height) {
        m_impl->setSize(width, height);
    }

    void WebviewEditor::setParent(void* parentHandle) {
        m_impl->setParent(parentHandle);
    }

    void WebviewEditor::show() {
        m_impl->show();
    }

    void WebviewEditor::hide() {
        m_impl->hide();
    }

} // namespace mostly_harmless::gui