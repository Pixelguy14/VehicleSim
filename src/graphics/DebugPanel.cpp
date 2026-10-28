#include "graphics/DebugPanel.h"

#include <cmath>
#include <cstddef>

#include <imgui.h>

#include "core/FixedStepper.h"
#include "math/Camera2D.h"

namespace vs {

void DebugPanel::PushFrameTime(float milliseconds) {
    if (historyCount_ < kHistorySize) {
        history_[historyCount_] = milliseconds;
        ++historyCount_;
        return;
    }

    // Ventana deslizante. Son 120 floats por frame: moverlos a mano es mas
    // barato que cualquier estructura circular y deja el vector contiguo, que es
    // lo que espera PlotLines.
    for (std::size_t i = 1; i < kHistorySize; ++i) {
        history_[i - 1] = history_[i];
    }
    history_[kHistorySize - 1] = milliseconds;
}

void DebugPanel::Draw(const DebugPanelContext& context) {
    ImGui::SetNextWindowPos(ImVec2(12.0F, 12.0F), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.92F);

    // AlwaysAutoResize en lugar de una altura fija: la seccion "PAUSADO" y la
    // linea de estadisticas solo existen a veces, y con un tamano fijo ImGui
    // metia una barra de scroll y cortaba la ultima linea de controles.
    if (!ImGui::Begin("VehicleSim", nullptr,
                      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (context.snapshot != nullptr && context.snapshot->fps > 0.0) {
        const FrameSnapshot& snapshot = *context.snapshot;

        ImGui::Text("%.1f fps", static_cast<double>(snapshot.fps));
        ImGui::SameLine();
        ImGui::TextDisabled("%.2f ms/frame", static_cast<double>(snapshot.frameMs));

        ImGui::Text("fisica %.3f ms | %.2f pasos/frame", static_cast<double>(snapshot.physicsMs),
                    static_cast<double>(snapshot.stepsPerFrame));
    } else if (historyCount_ > 0) {
        // Estadistica por segundo todavia no disponible, pero el ultimo frame ya
        // es un dato: mostrarlo evita que el panel parezca congelado al arrancar.
        ImGui::TextDisabled("midiendo...");
        ImGui::SameLine();
        ImGui::Text("%.2f ms/frame", static_cast<double>(history_[historyCount_ - 1]));
    } else {
        ImGui::TextDisabled("midiendo...");
    }

    // Escala fija de 0 a 25 ms: si se autoscale, un solo pico aplasta el resto de
    // la grafica y deja de verse si el frame medio es 8 ms o 12 ms. 25 ms deja
    // los 8.3 de 120 Hz en un tercio de la altura y los 16.7 de 60 Hz al 60%.
    ImGui::PlotLines("##tiempo", history_.data(), static_cast<int>(historyCount_), 0, "ms/frame",
                     0.0F, 25.0F, ImVec2(-1.0F, 64.0F));

    if (context.stepper != nullptr) {
        ImGui::SeparatorText("Simulacion");
        FixedStepper& stepper = *context.stepper;

        // Ancho fijo para los sliders: con el ancho automatico, la etiqueta de la
        // derecha se come el control y queda un deslizador de tres pixeles.
        ImGui::PushItemWidth(150.0F);

        int hertz = static_cast<int>(std::lround(1.0 / stepper.FixedDelta()));
        if (ImGui::SliderInt("Paso fijo", &hertz, 30, 240, "%d Hz")) {
            stepper.SetFixedDelta(1.0 / static_cast<double>(hertz));
        }

        int maxSteps = stepper.MaxStepsPerFrame();
        if (ImGui::SliderInt("Max pasos/frame", &maxSteps, 1, 16)) {
            stepper.SetMaxStepsPerFrame(maxSteps);
        }

        ImGui::PopItemWidth();

        if (ImGui::Button("Descartar tiempo", ImVec2(210.0F, 0.0F))) {
            stepper.Reset();
        }
    }

    if (context.camera != nullptr || context.cameraPosition != nullptr) {
        ImGui::SeparatorText("Camara");
        ImGui::PushItemWidth(150.0F);

        if (context.cameraPosition != nullptr) {
            float position[2] = {context.cameraPosition->x, context.cameraPosition->y};
            if (ImGui::DragFloat2("Posicion", position, 0.25F, -1000.0F, 1000.0F, "%.1f m")) {
                *context.cameraPosition = Vec2{position[0], position[1]};
            }
        }

        if (context.camera != nullptr) {
            float zoom = context.camera->Zoom();
            if (ImGui::SliderFloat("Zoom", &zoom, 5.0F, 200.0F, "%.0f px/m")) {
                context.camera->SetZoom(zoom);
            }
        }

        ImGui::PopItemWidth();
    }

    if (context.showGrid != nullptr || context.paused != nullptr) {
        ImGui::SeparatorText("Vista");

        if (context.showGrid != nullptr) {
            ImGui::Checkbox("Grilla", context.showGrid);
        }
        if (context.paused != nullptr) {
            ImGui::Checkbox("Pausa", context.paused);
        }
    }

    if (context.paused != nullptr && *context.paused) {
        ImGui::TextColored(ImVec4(0.95F, 0.70F, 0.25F, 1.0F), "PAUSADO");
    }

    ImGui::SeparatorText("Controles");
    ImGui::TextDisabled("F1 panel | F2 pausa | Esc salir");
    ImGui::TextDisabled("WASD camara | Espacio x3");

    ImGui::End();
}

}  // namespace vs
