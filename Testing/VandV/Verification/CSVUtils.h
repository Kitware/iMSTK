/* Distributed under the Apache License, Version 2.0.
   See accompanying NOTICE file for details.*/

#pragma once

#include <limits>
#include <string>
#include <vector>
#include <map>

namespace imstk
{
using CSV = std::vector<std::pair<std::string, std::vector<float>>>;

bool readCSV(std::string const& filename, CSV& data);
bool writeCSV(std::string const& filename, const CSV& data);

struct HeaderSummary
{
    size_t size = 0;
    double sum  = 0;
    double mean = 0;
    double min  = std::numeric_limits<float>::max();
    double max  = -std::numeric_limits<float>::max();
    double rootMeanSquared = 0;

    size_t numErrors  = 0;
    float minError    = std::numeric_limits<float>::max();
    float maxError    = -std::numeric_limits<float>::max();
    double errorStart = -1;
    double errorEnd   = -1;
};
using CSVSummary = std::map<std::string, HeaderSummary>;

struct CSVComparison
{
    std::string name;
    double percrentDifference = 2.0;
    bool passed = true;
    CSVSummary computedSummary;
};

bool compareCSVs(const CSV& baseline, const CSV& computed, CSVComparison& cmp);
bool compareCSVs(std::string const& baseline, std::string const& computed, CSVComparison& cmp);
bool compareAndPlotTimeSeriesCSVs(std::string const& baseline, std::string const& computed, CSVComparison& cmp, std::string const& outDir, bool onlyPlotOnError = true);

struct PointPlotConfig
{
    double dt;
    std::string baseName = "";
    std::string xPrefix  = "";
    std::string xPostfix = "";
    std::string yPrefix  = "";
    std::string yPostfix = "";
    bool constYAxis      = false;
    std::vector<double> times;
    std::vector<double> xRootMeanSquared;
    std::vector<double> yRootMeanSquared;
};
bool plot2DPointCSVs(std::string const& baseline, std::string const& computed, PointPlotConfig& cfg, std::string const& outDir);

double percentDifference(double expected, double calculated, double epsilon = 1E-20);
double percentTolerance(double expected, double calculated, double epsilon  = 1E-20);

struct CSVPlotSource
{
    CSVPlotSource(std::string const& name, const CSV& data,
                  double red, double green, double blue, float width, int pen_type = 1) : csv(data), name(name)
    {
        r = red;
        g = green;
        b = blue;
        w = width;
        p = pen_type;
    }

    ~CSVPlotSource() = default;
    const CSV& csv;
    double r = 0, g = 0, b = 0;
    float w = 2;
    int p   = 1;
    std::string name;
};
bool plotTimeSeriesCSV(std::string const& base_name, std::vector<CSVPlotSource> const& srcs);
bool plotTimeSeriesCSV(std::string const& base_name, std::vector<CSVPlotSource> const& srcs, bool const_y_axis);

bool pointPlot2DCSVs(std::string const& base_name, std::vector<CSVPlotSource> const& srcs, PointPlotConfig& cfg);
} // namespace imstk
