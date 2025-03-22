#include "synth_Editor.h"
#include <mostlyharmless_SynthWebResources.h>
#include <nlohmann/json.hpp>

[[nodiscard]] static mostly_harmless::gui::WebviewEditor::Resource createResourceFor(const std::string& name) {
    auto resOpt = mostly_harmless::SynthWebResources::getNamedResource(name);
    assert(resOpt);
    auto [data, size] = *resOpt;
    auto mimeType = mostly_harmless::gui::getMimeType(name);
    assert(mimeType);
    auto* asUnsigned = reinterpret_cast<const std::uint8_t*>(data);
    mostly_harmless::gui::WebviewEditor::Resource temp;
    temp.data = { asUnsigned, asUnsigned + size };
    temp.mimeType = *mimeType;
    return temp;
}

Editor::Editor(SharedState* sharedState) : mostly_harmless::gui::WebviewEditor(sharedState, 500, 500, mostly_harmless::gui::Colour{ 0xFF000000 }),
                                           m_sharedState(sharedState) {
    std::stringstream initialDataStream;
    nlohmann::json j;
    auto rawParamView = sharedState->getRawParameterView();
    for (auto& p : rawParamView) {
        auto& el = j[p.name];
        el["pid"] = p.pid;
        const auto [min, max] = p.range;
        el["min"] = min;
        el["max"] = max;
        el["initial"] = p.value;
    }
    initialDataStream << "window.params = " << j << ";";

#if defined(HOT_RELOAD)
    this->setOptions({ .enableDebug = true, .initScript = initialDataStream.str() });
#else
    m_resources.emplace("/index.html", createResourceFor("index.html"));
    m_resources.emplace("/index.css", createResourceFor("index.css"));
    m_resources.emplace("/index.js", createResourceFor("index.js"));

    this->setOptions({
        .enableDebug = true,
        .contentProvider = [this](const std::string& name) -> std::optional<mostly_harmless::gui::WebviewEditor::Resource> {
            const auto requested = name == "/" ? "/index.html" : name;
            const auto it = m_resources.find(requested);
            if (it == m_resources.end()) return {};
            auto resource = it->second;
            return resource;
        },
        .initScript = initialDataStream.str(),
    });

#endif
}


void Editor::initialise() {
    mostly_harmless::gui::WebviewEditor::initialise();
#if defined(HOT_RELOAD)
    m_internalWebview->navigate("http://localhost:5173");
#endif
}
