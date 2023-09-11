/* Distributed under the Apache License, Version 2.0.
   See accompanying NOTICE file for details.*/

#include "CSVUtils.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <cctype>
#include <locale>

#include "imstkLogger.h"

#include "vtkAxis.h"
#include "vtkDataSet.h"
#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkChartXY.h"
#include "vtkChartLegend.h"
#include "vtkContextView.h"
#include "vtkContextScene.h"
#include "vtkDataSetReader.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkGeometryFilter.h"
#include "vtkGraphicsFactory.h"
#include "vtkIntArray.h"
#include "vtkPlot.h"
#include "vtkTextProperty.h"
#include "vtkPNGWriter.h"
#include "vtkPen.h"
#include "vtkPointData.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkResampleWithDataSet.h"
#include "vtkSmartPointer.h"
#include "vtkTable.h"
#include "vtkThreshold.h"
#include "vtkTriangleFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkWindowToImageFilter.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkXMLReader.h"
#include "vtksys/SystemTools.hxx"

namespace imstk
{
// trim from start (in place)
static inline void
ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void
rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void
trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

bool
writeCSV(std::string const& filename, const CSV& data)
{
    std::ofstream csv;
    csv.open(filename, std::ofstream::out | std::ofstream::trunc);
    if (!csv.good())
    {
        return false;
    }

    size_t num_rows    = 0;
    size_t num_columns = data.size();
    // Write out the headers
    for (auto columns : data)
    {
        num_columns--;
        csv << columns.first;
        if (num_columns > 0)
        {
            csv << ", ";
        }
        num_rows = columns.second.size();
    }
    csv << "\n";

    // Write out the data
    for (size_t idx = 0; idx < num_rows; idx++)
    {
        // Write out values
        num_columns = data.size();
        for (auto columns : data)
        {
            num_columns--;
            auto& v = columns.second;
            csv << v[idx];
            if (num_columns > 0)
            {
                csv << ", ";
            }
        }
        csv << "\n";
    }
    csv << "\n";
    csv.close();
    return true;
}

bool
readCSV(std::string const& filename, CSV& data)
{
    data.clear();
    std::ifstream csv;
    csv.open(filename, std::ofstream::in);
    if (!csv.good())
    {
        return false;
    }

    int         line_num = 1;
    int         idx;
    bool        get_names = true;
    std::string line;
    while (!csv.eof()) // To get you all the lines.
    {
        idx = 0;
        std::getline(csv, line); // Get the line.
        if (line.empty())
        {
            break;
        }
        std::stringstream s_stream(line); // Create stringstream from the line
        while (s_stream.good())
        {
            std::string value;
            std::getline(s_stream, value, ','); //get first string delimited by comma
            if (get_names)
            {
                trim(value);
                data.push_back(std::pair<std::string, std::vector<float>>(value, std::vector<float>()));
            }
            else
            {
                try
                {
                    data[idx++].second.push_back((float)std::stod(value));
                }
                catch (...)
                {
                    return false;
                }
            }
        }
        line_num++;
        get_names = false;
    }

    return true;
}

double
percentTolerance(double expected, double calculated, double epsilon)
{
    // Check for 'invalid' numbers first
    //if (Double.isNaN(expected) || Double.isNaN(calculated) || Double.isInfinite(expected) || Double.isInfinite(calculated))
    //  Log.warn("While finding percent tolerance from values 'expected' = " + expected + " and 'calculated' =" + calculated +
    //    ", invalid values (NaN or Infinity) were found.  Unexpected results may occur.");
    // Handle special cases
    if (expected == 0.0 && calculated == 0.0)
    {
        return 0.0;
    }
    else if (expected == 0.0 || calculated == 0.0)
    {
        if (std::fabs(expected + calculated) < epsilon)
        {
            return 0.0;
        }
        else
        {
            if (expected == 0.0)
            {
                return std::numeric_limits<double>::infinity();
            }
            else if (expected < 0.0)
            {
                return -100.0;
            }
            else
            {
                return 100.0;
            }
        }
    }
    else
    {
        return std::fabs(calculated - expected) / expected * 100.0;
    }
}

double
percentDifference(double expected, double calculated, double epsilon)
{
    // Check for 'invalid' numbers first
    //if (Double.isNaN(expected) || Double.isNaN(calculated) || Double.isInfinite(expected) || Double.isInfinite(calculated))
    //  Log.warn("While finding percent difference from values 'expected' = " + expected + " and 'calculated' =" + calculated +
    //    ", invalid values (NaN or Infinity) were found.  Unexpected results may occur.");
    // Handle special cases
    if (expected == 0.0 && calculated == 0.0)
    {
        return 0.0;
    }
    else if (expected == 0.0 || calculated == 0.0)
    {
        if (std::fabs(expected + calculated) < epsilon)
        {
            return 0.0;
        }
        else
        {
            return 200.0;
        }
    }
    else
    {
        double difference = (calculated - expected);
        double average    = (calculated + expected) / 2.0;

        if (average == 0.0)
        {
            return std::numeric_limits<double>::infinity();
        }

        return std::fabs(difference / average) * 100.0;
    }
}

double
RMSError(double expected, double calculated, double epsilon)
{
    // Check for 'invalid' numbers first
    //if (Double.isNaN(expected) || Double.isNaN(calculated) || Double.isInfinite(expected) || Double.isInfinite(calculated))
    //  Log.warn("While finding percent difference from values 'expected' = " + expected + " and 'calculated' =" + calculated +
    //    ", invalid values (NaN or Infinity) were found.  Unexpected results may occur.");
    // Handle special cases
    if (expected == 0.0 && calculated == 0.0)
    {
        return 0.0;
    }
    else if (expected == 0.0 || calculated == 0.0)
    {
        if (std::fabs(expected + calculated) < epsilon)
        {
            return 0.0;
        }
        else
        {
            return 200.0;
        }
    }
    else
    {
        double difference = (calculated - expected);
        double average    = (calculated + expected) / 2.0;

        if (average == 0.0)
        {
            return std::numeric_limits<double>::infinity();
        }

        return std::fabs(difference / average) * 100.0;
    }
}

bool
compareCSVs(std::string const& baseline, std::string const& computed, CSVComparison& cmp)
{
    CSV b;
    CSV c;
    if (!readCSV(baseline, b))
    {
        LOG(FATAL) << "Unable to read CSV file " << baseline;
        return false;
    }
    if (!readCSV(computed, c))
    {
        LOG(FATAL) << "Unable to read CSV file " << computed;
        return false;
    }

    return compareCSVs(b, c, cmp);
}

bool
compareAndPlotTimeSeriesCSVs(std::string const& baseline, std::string const& computed, CSVComparison& cmp, std::string const& outDir, bool onlyPlotOnError)
{
    bool same = true;
    vtksys::SystemTools::MakeDirectory(outDir);
    // Read in the data
    CSV b;
    CSV c;
    if (!readCSV(baseline, b))
    {
        LOG(FATAL) << "Unable to read CSV file " << baseline;
        return false;
    }
    if (!readCSV(computed, c))
    {
        LOG(FATAL) << "Unable to read CSV file " << computed;
        return false;
    }

    compareCSVs(b, c, cmp);

    if ((onlyPlotOnError && !cmp.passed) || !onlyPlotOnError)
    {
        // Plot the data from these 2 files
        std::vector<CSVPlotSource> plot_sources;
        plot_sources.push_back(CSVPlotSource("Baseline", b, 255 / 255, 194 / 255, 10 / 255, 2.f, vtkPen::SOLID_LINE));
        plot_sources.push_back(CSVPlotSource("Computed", c, 12 / 255, 123 / 255, 220 / 255, 5.f, vtkPen::DASH_LINE));
        plotTimeSeriesCSV(outDir, plot_sources, false);
    }
    else
    {
        LOG(INFO) << "Comparison between " << baseline << " and " << computed << " passed. Not Plotting.";
    }

    return same;
}

bool
compareCSVs(const CSV& baseline, const CSV& computed, CSVComparison& cmp)
{
    double diff;
    size_t num_columns = baseline.size();
    if (computed.size() != num_columns)
    {
        LOG(FATAL) << "All CSV's need to have the same number of columns";
        return false;
    }

    // Make sure all the CSV's have the same number of columns, column names, and value lengths
    for (size_t c = 1; c < num_columns; c++)
    {
        auto& bCol = baseline[c];
        auto& cCol = computed[c];

        std::string header = bCol.first;
        if (header != cCol.first)
        {
            LOG(FATAL) << "CSVPlotSource column names do not match";
            return false;
        }

        auto& bData = bCol.second;
        auto& cData = cCol.second;
        if (bData.size() != cData.size())
        {
            LOG(FATAL) << "CSVPlotSource columns size does not match";
            return false;
        }

        auto& summary = cmp.computedSummary[header];
        summary.size = cData.size();
        bool previousError = false;
        for (size_t i = 0; i < bData.size(); i++)
        {
            double cValue = cData[i];
            summary.sum += cValue;
            if (cValue < summary.min)
            {
                summary.min = cValue;
            }
            if (cValue > summary.maxError)
            {
                summary.max = cValue;
            }
            diff = percentDifference(bData[i], cValue);
            if (diff < summary.minError)
            {
                summary.minError = diff;
            }
            if (diff > summary.maxError)
            {
                summary.maxError = diff;
            }
            if (diff > cmp.percrentDifference)
            {
                summary.numErrors++;
                previousError = true;
                if (summary.errorStart == -1)
                {
                    summary.errorStart = baseline[0].second[i];
                }
            }
            else if (previousError)
            {
                summary.errorEnd = baseline[0].second[i];
                previousError    = false;
            }
        }

        if (summary.errorStart != -1 && summary.errorEnd == -1)
        {
            summary.errorEnd = baseline[0].second[summary.size];
        }
        summary.mean = summary.sum / summary.size;

        for (size_t i = 1; i < bData.size(); i++)
        {
            summary.rootMeanSquared += (bData[i] - cData[i]) * (bData[i] - cData[i]);
        }
        summary.rootMeanSquared /= bData.size();
        summary.rootMeanSquared  = sqrt(summary.rootMeanSquared);
        if (summary.numErrors > 0)
        {
            cmp.passed = false;
        }
    }
    return true;
}

bool
plotTimeSeriesCSV(std::string const& base_name, std::vector<CSVPlotSource> const& srcs, bool const_y_axis)
{
    if (srcs.empty())
    {
        return false;
    }

    size_t num_columns = srcs[0].csv.size();
    size_t num_values  = srcs[0].csv[0].second.size();
    float  y_min       = std::numeric_limits<float>::max();
    float  y_max       = -std::numeric_limits<float>::max();

    //Set Graph Properties
    int titleFont  = 26;
    int axisFont   = 24;
    int legendFont = 26;
    // Make sure all the CSV's have the same number of columns, column names, and value lengths

    for (size_t i = 0; i < srcs.size(); i++)
    {
        auto& src = srcs[i];
        if (src.csv.size() != num_columns)
        {
            LOG(FATAL) << "All CSV's need to have the same number of columns";
            return false;
        }
        // Make sure all the columns are the same
        for (size_t c = 1; c < num_columns; c++)
        {
            auto& csv = src.csv[c];
            if (csv.first != srcs[0].csv[c].first)
            {
                LOG(FATAL) << "CSVPlotSource column names do not match";
                return false;
            }
            if (csv.second.size() != srcs[0].csv[c].second.size())
            {
                LOG(FATAL) << "CSVPlotSource columns size does not match";
                return false;
            }

            if (const_y_axis)
            {
                // Make sure all our plots have the same y scale
                for (size_t v = 0; v < csv.second.size(); v++)
                {
                    float y_val = csv.second[v];
                    if (y_val > y_max)
                    {
                        y_max = y_val;
                    }
                    if (y_val < y_min)
                    {
                        y_min = y_val;
                    }
                }
            }
        }
    }
    if (const_y_axis)
    {
        // Add 2% on each side
        float pad = (std::fabs(y_min) + std::fabs(y_max)) * 0.02;
        y_min -= pad;
        y_max += pad;
    }

    for (size_t c = 1; c < num_columns; c++)
    {
        std::string plot_name = base_name + srcs[0].csv[c].first;

        auto table     = vtkSmartPointer<vtkTable>::New();
        auto time_data = vtkSmartPointer<vtkFloatArray>::New();
        time_data->SetName("X-Axis");
        table->AddColumn(time_data);
        // Create Columns for each line
        for (size_t s = 0; s < srcs.size(); s++)
        {
            auto y_values = vtkSmartPointer<vtkFloatArray>::New();
            y_values->SetName(srcs[s].name.c_str());
            table->AddColumn(y_values);
        }
        // Add data to each column (must be done after all columns are created)
        table->SetNumberOfRows(num_values);
        for (size_t s = 0; s < srcs.size(); s++)
        {
            const CSV& csv = srcs[s].csv;
            for (size_t y = 0; y < num_values; y++)
            {
                if (s == 0)// Add X values from the first axis
                {
                    table->SetValue(y, 0, csv[0].second[y]);
                }
                table->SetValue(y, s + 1, csv[c].second[y]);
            }
        }

        // Create our chart
        auto chart = vtkSmartPointer<vtkChartXY>::New();
        //chart->SetTitle(srcs[0].csv[0].first + " vs. " + srcs[0].csv[c].first);
        chart->GetAxis(vtkAxis::LEFT)->SetTitle(srcs[0].csv[c].first);
        if (const_y_axis)
        {
            chart->GetAxis(vtkAxis::LEFT)->SetMinimum(y_min);
            chart->GetAxis(vtkAxis::LEFT)->SetMaximum(y_max);
            chart->GetAxis(vtkAxis::LEFT)->SetBehavior(vtkAxis::FIXED);
        }
        chart->GetAxis(vtkAxis::BOTTOM)->SetTitle(srcs[0].csv[0].first);
        chart->GetAxis(vtkAxis::BOTTOM)->GetTitleProperties()->SetFontSize(titleFont);
        chart->GetAxis(vtkAxis::LEFT)->GetTitleProperties()->SetFontSize(titleFont);

        chart->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetFontSize(axisFont);
        chart->GetAxis(vtkAxis::LEFT)->GetLabelProperties()->SetFontSize(axisFont);

        // Add some lines to the chart
        for (size_t s = 0; s < srcs.size(); s++)
        {
            auto&    src    = srcs[s];
            vtkPlot* c_line = chart->AddPlot(vtkChart::LINE);
            c_line->SetInputData(table, 0, s + 1);// Always using Column 0 as our x coordinate
            c_line->SetColor(src.r, src.g, src.b);
            c_line->SetWidth(src.w);
            c_line->GetPen()->SetLineType(src.p);
        }
        // Add the Legend
        // Set up the legend to be off to the top right of the viewport.
        chart->SetShowLegend(true);
        chart->GetLegend()->SetInline(false);
        chart->GetLegend()->SetHorizontalAlignment(vtkChartLegend::CENTER);
        chart->GetLegend()->SetVerticalAlignment(vtkChartLegend::TOP);
        chart->GetLegend()->GetLabelProperties()->SetFontSize(legendFont);

        // Set up the view
        auto view = vtkSmartPointer<vtkContextView>::New();
        view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
        view->GetRenderer()->SetBackgroundAlpha(1.0);
        view->GetRenderWindow()->SetSize(1024, 768);
        view->GetRenderWindow()->SetOffScreenRendering(1);
        view->GetScene()->AddItem(chart);
        view->GetRenderWindow()->Render();
        // Screenshot
        auto windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
        windowToImageFilter->SetInput(view->GetRenderWindow());
        windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
        windowToImageFilter->ReadFrontBufferOff();       // read from the back buffer
        windowToImageFilter->Update();
        // Write to disk
        LOG(INFO) << "Writing " + plot_name;
        auto writer = vtkSmartPointer<vtkPNGWriter>::New();
        writer->SetFileName(std::string(plot_name + ".png").c_str());
        writer->SetInputConnection(windowToImageFilter->GetOutputPort());
        writer->Write();
    }
    return true;
}

bool
plot2DPointCSVs(std::string const& baseline, std::string const& computed, PointPlotConfig& cfg, std::string const& outDir)
{
    vtksys::SystemTools::MakeDirectory(outDir);
    // Read in the data

    CSV b;
    CSV c;
    if (!readCSV(baseline, b))
    {
        LOG(FATAL) << "Unable to read CSV file " << baseline;
        return false;
    }
    if (!readCSV(computed, c))
    {
        LOG(FATAL) << "Unable to read CSV file " << computed;
        return false;
    }

    // Plot the data from these 2 files
    std::vector<CSVPlotSource> plot_sources;
    plot_sources.push_back(CSVPlotSource("Baseline", b, 0.0, 0.0, 0.0, 5.f, vtkPen::SOLID_LINE));
    plot_sources.push_back(CSVPlotSource("Computed", c, 1.0, 0.0, 0.0, 2.f, vtkPen::SOLID_LINE));
    return pointPlot2DCSVs(outDir, plot_sources, cfg);
}

bool
pointPlot2DCSVs(std::string const& base_name, std::vector<CSVPlotSource> const& srcs, PointPlotConfig& cfg)
{
    if (srcs.empty())
    {
        return false;
    }

    std::vector<std::pair<size_t, size_t>> pointColumns;

    size_t num_columns = srcs[0].csv.size();
    float  y_min       = std::numeric_limits<float>::max();
    float  y_max       = -std::numeric_limits<float>::max();

    //Set Graph Properties
    int titleFont  = 26;
    int axisFont   = 24;
    int legendFont = 26;

    // Make sure all the CSV's have the same number of columns, column names, and value lengths
    for (size_t i = 0; i < srcs.size(); i++)
    {
        auto& src = srcs[i];
        if (src.csv.size() != num_columns)
        {
            LOG(FATAL) << "All CSV's need to have the same number of columns";
            return false;
        }
        // Make sure all the columns are the same
        for (size_t c = 1; c < num_columns; c++)
        {
            auto&       csv    = src.csv[c];
            std::string header = csv.first;
            if (header != srcs[0].csv[c].first)
            {
                LOG(FATAL) << "CSVPlotSource column names do not match";
                return false;
            }
            if (csv.second.size() != srcs[0].csv[c].second.size())
            {
                LOG(FATAL) << "CSVPlotSource columns size does not match";
                return false;
            }

            if (cfg.constYAxis)
            {
                // Make sure all our plots have the same y scale
                for (size_t v = 0; v < csv.second.size(); v++)
                {
                    float y_val = csv.second[v];
                    if (y_val > y_max)
                    {
                        y_max = y_val;
                    }
                    if (y_val < y_min)
                    {
                        y_min = y_val;
                    }
                }
            }

            // I am assuming the x and y columns are next to each other
            // And the csv columns are in the same order
            if (i == 0 && header.find_first_of(cfg.xPrefix) == 0 && header.find(cfg.xPostfix) != std::string::npos)
            {
                pointColumns.push_back(std::pair<size_t, size_t>(c, c + 1));
            }
        }
    }
    if (cfg.constYAxis)
    {
        // Add 2% on each side
        float pad = (std::fabs(y_min) + std::fabs(y_max)) * 0.02;
        y_min -= pad;
        y_max += pad;
    }

    bool                addRMS = srcs.size() == 2;
    std::vector<double> xDists; // For RMS
    std::vector<double> yDists; // For RMS
    for (double t : cfg.times)
    {
        size_t      idx       = t / cfg.dt;
        std::string plot_name = base_name + cfg.baseName + std::to_string(t) + "s";

        auto table = vtkSmartPointer<vtkTable>::New();
        // Create 2 Columns (for x and y coord) for each source
        for (size_t s = 0; s < srcs.size(); s++)
        {
            auto x_values = vtkSmartPointer<vtkFloatArray>::New();
            x_values->SetName(std::string(srcs[s].name + "_" + cfg.xPrefix + cfg.xPostfix).c_str());
            table->AddColumn(x_values);

            auto y_values = vtkSmartPointer<vtkFloatArray>::New();
            y_values->SetName(std::string(srcs[s].name + "_" + cfg.yPrefix + cfg.yPostfix).c_str());
            table->AddColumn(y_values);
        }
        // Add data to each column (must be done after all columns are created)
        size_t table_col = 0;
        xDists.clear();
        yDists.clear();

        table->SetNumberOfRows(pointColumns.size());
        for (size_t s = 0; s < srcs.size(); s++)
        {
            const CSV& csv = srcs[s].csv;
            for (size_t p = 0; p < pointColumns.size(); p++)
            {
                double x = csv[pointColumns[p].first].second[idx];
                double y = csv[pointColumns[p].second].second[idx];
                table->SetValue(p, table_col, x);
                table->SetValue(p, table_col + 1, y);

                // I am using these to compute RMS and am assuming there are only 2 sources
                if (addRMS)
                {
                    if (s == 0)
                    {
                        xDists.push_back(x);
                        yDists.push_back(y);
                    }
                    else
                    {
                        xDists[p] = std::pow(xDists[p] - x, 2);
                        yDists[p] = std::pow(yDists[p] - y, 2);
                    }
                }
            }
            table_col += 2;
        }
        // Calculate x and y RMS values
        std::string RMS = "";
        if (addRMS)
        {
            double xRMS = 0;
            double yRMS = 0;
            for (size_t p = 0; p < pointColumns.size(); p++)
            {
                xRMS += xDists[p];
                yRMS += yDists[p];
            }
            xRMS /= pointColumns.size();
            xRMS  = sqrt(xRMS);
            yRMS /= pointColumns.size();
            yRMS  = sqrt(yRMS);
            RMS   = ", xRMS = " + std::to_string(xRMS) + ", yRMS = " + std::to_string(yRMS);
            cfg.xRootMeanSquared.push_back(xRMS);
            cfg.yRootMeanSquared.push_back(yRMS);
        }

        //Create our chart
        auto chart = vtkSmartPointer<vtkChartXY>::New();
        chart->SetTitle(std::to_string(t) + "s" + RMS);
        chart->GetAxis(vtkAxis::LEFT)->SetTitle(cfg.yPrefix + cfg.yPostfix);
        if (cfg.constYAxis)
        {
            chart->GetAxis(vtkAxis::LEFT)->SetMinimum(y_min);
            chart->GetAxis(vtkAxis::LEFT)->SetMaximum(y_max);
            chart->GetAxis(vtkAxis::LEFT)->SetBehavior(vtkAxis::FIXED);
        }
        chart->GetAxis(vtkAxis::BOTTOM)->SetTitle(cfg.xPrefix + cfg.xPostfix);

        chart->GetAxis(vtkAxis::BOTTOM)->GetTitleProperties()->SetFontSize(titleFont);
        chart->GetAxis(vtkAxis::LEFT)->GetTitleProperties()->SetFontSize(titleFont);

        chart->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetFontSize(axisFont);
        chart->GetAxis(vtkAxis::LEFT)->GetLabelProperties()->SetFontSize(axisFont);

        // Add some points to the chart
        table_col = 0;
        for (size_t s = 0; s < srcs.size(); s++)
        {
            auto&    src    = srcs[s];
            vtkPlot* c_line = chart->AddPlot(vtkChart::POINTS);
            c_line->SetInputData(table, table_col, table_col + 1);
            c_line->SetColor(src.r, src.g, src.b);
            c_line->SetWidth(src.w);
            c_line->GetPen()->SetLineType(src.p);
            table_col += 2;
        }
        // Add the Legend
        // Set up the legend to be off to the top right of the viewport.
        chart->SetShowLegend(true);
        chart->GetLegend()->SetInline(false);
        chart->GetLegend()->SetHorizontalAlignment(vtkChartLegend::CENTER);
        chart->GetLegend()->SetVerticalAlignment(vtkChartLegend::TOP);
        chart->GetLegend()->GetLabelProperties()->SetFontSize(legendFont);

        // Set up the view
        auto view = vtkSmartPointer<vtkContextView>::New();
        view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
        view->GetRenderer()->SetBackgroundAlpha(1.0);
        view->GetRenderWindow()->SetSize(1024, 768);
        view->GetRenderWindow()->SetOffScreenRendering(1);
        view->GetScene()->AddItem(chart);
        view->GetRenderWindow()->Render();
        // Screenshot
        auto windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
        windowToImageFilter->SetInput(view->GetRenderWindow());
        windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
        windowToImageFilter->ReadFrontBufferOff();       // read from the back buffer
        windowToImageFilter->Update();
        // Write to disk
        LOG(INFO) << "Writing " + plot_name;
        auto writer = vtkSmartPointer<vtkPNGWriter>::New();
        writer->SetFileName(std::string(plot_name + ".png").c_str());
        writer->SetInputConnection(windowToImageFilter->GetOutputPort());
        writer->Write();
    }
    return true;
}
} // namespace imstk
