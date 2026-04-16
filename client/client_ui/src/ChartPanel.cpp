#include "client_ui/ChartPanel.h"
#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>
#include <algorithm>
#include <vector>
#include <cstring>
#include "common/Instrument.h"

namespace client_ui {

    ChartPanel::ChartPanel() {}

    void ChartPanel::onCandleUpdate(int interval, const ohlc::Candle& candle) {
        if (interval != mInterval || candle.symbol != mSymbol) return;

        std::lock_guard<std::mutex> lock(mMutex);
        
        // Check if we just update the last one or add new
        if (!mCandles.empty() && mCandles.back().timestamp == candle.timestamp) {
            mCandles.back() = candle;
        } else {
            mCandles.push_back(candle);
            if (mCandles.size() > 500) mCandles.erase(mCandles.begin());
        }
    }

    void ChartPanel::render() {
        ImGui::Begin("Market Chart");
        
        ImGui::SetNextItemWidth(100);
        
        static int selectedSymbolIdx = 0;
        const char* symbolNames[common::symbol_names.size()];
        for(size_t i = 0; i < common::symbol_names.size(); ++i) {
            symbolNames[i] = common::symbol_names[i].data();
        }
        
        if (ImGui::Combo("Symbol", &selectedSymbolIdx, symbolNames, common::symbol_names.size())) {
            strncpy(mSymbol, symbolNames[selectedSymbolIdx], sizeof(mSymbol) - 1);
            mSymbol[sizeof(mSymbol) - 1] = '\0';
            // In a real app, trigger a fetch of historical data here
            std::lock_guard<std::mutex> lock(mMutex);
            mCandles.clear();
        }
        
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        if (ImGui::Combo("Period", &mInterval, "1m\0 5m\0\0")) {
            // In a real app, clear and reload from DB here
        }

        drawCandleChart();

        ImGui::End();
    }

    // Custom Candlestick Plotter
    void PlotCandlestick(const char* label_id, const double* xs, const double* opens, const double* closes, const double* lows, const double* highs, int count, float width_percent) {
        ImDrawList* draw_list = ImPlot::GetPlotDrawList();
        double half_width = (count > 1) ? (xs[1] - xs[0]) * width_percent : 0.5;

        if (ImPlot::BeginItem(label_id)) {
            if (ImPlot::FitThisFrame()) {
                for (int i = 0; i < count; ++i) {
                    ImPlot::FitPoint(ImPlotPoint(xs[i], lows[i]));
                    ImPlot::FitPoint(ImPlotPoint(xs[i], highs[i]));
                }
            }

            for (int i = 0; i < count; ++i) {
                ImVec2 open_pos  = ImPlot::PlotToPixels(xs[i] - half_width, opens[i]);
                ImVec2 close_pos = ImPlot::PlotToPixels(xs[i] + half_width, closes[i]);
                ImVec2 low_pos   = ImPlot::PlotToPixels(xs[i], lows[i]);
                ImVec2 high_pos  = ImPlot::PlotToPixels(xs[i], highs[i]);

                ImU32 color = (opens[i] > closes[i]) ? IM_COL32(239, 83, 80, 255) : IM_COL32(38, 166, 154, 255);
                draw_list->AddLine(low_pos, high_pos, color);
                draw_list->AddRectFilled(open_pos, close_pos, color);
            }
            ImPlot::EndItem();
        }
    }

    void ChartPanel::drawCandleChart() {
        std::lock_guard<std::mutex> lock(mMutex);
        
        if (mCandles.empty()) {
            ImGui::Text("No data for chart.");
            return;
        }

        if (ImPlot::BeginPlot("##OHLC", ImVec2(-1, -1), ImPlotFlags_None)) {
            ImPlot::SetupAxes("Time", "Price", ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            ImPlot::SetupAxisLimits(ImAxis_Y1, mCandles.back().low - 0.01, mCandles.back().high + 0.01, ImGuiCond_Once);

            size_t count = mCandles.size();
            std::vector<double> xs(count), opens(count), highs(count), lows(count), closes(count);

            for (size_t i = 0; i < count; ++i) {
                xs[i] = (double)mCandles[i].timestamp;
                opens[i] = mCandles[i].open;
                highs[i] = mCandles[i].high;
                lows[i] = mCandles[i].low;
                closes[i] = mCandles[i].close;
            }

            PlotCandlestick("##Data", xs.data(), opens.data(), closes.data(), lows.data(), highs.data(), (int)count, 0.45f);

            ImPlot::EndPlot();
        }
    }

} // namespace client_ui
