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

///
/// \brief Reads a file of Comma Separated Values (CSV) into an STL structure
/// Each column of the CSV file is expected to have a string header
/// Column names are used as the key for a vector containing data values for that header
/// It is assumed that each header/column contains the same amount of values
/// It is assumed that the first header/column contains a time for the provided values
///
bool readCSV(std::string const& filename, CSV& data);

///
/// \brief Writes a file of Comma Separated Values (CSV) into an STL structure
/// Header strings will be written to the first row of the file
/// Values, at the same index from all vectors, are written row by row
/// It is assumed that all vectors are the same length
/// It is assumed that the first header/column contains a time for the provided values
///
bool writeCSV(std::string const& filename, const CSV& data);

///
/// \struct HeaderSummary
///
/// \brief Contains values of various statistics between two compared vectors of numbers
///
struct HeaderSummary
{
    size_t size = 0;
    double sum  = 0;
    double mean = 0;
    double min  = std::numeric_limits<float>::max();
    double max  = -std::numeric_limits<float>::max();
    double rootMeanSquared = 0;

    size_t numErrors  = 0;
    float  minError    = std::numeric_limits<float>::max();
    float  maxError    = -std::numeric_limits<float>::max();
    double errorStart = -1;
    double errorEnd   = -1;
};
using CSVSummary = std::map<std::string, HeaderSummary>;

///
/// \struct CSVComparison
///
/// \brief Contains the calculated statistics between all column values between two CSV files
/// A HeaderSummary describing the error statistics of the comparison for each of the same named colums
/// The passed flag is set to True if any values exceeded the provided percentDifference during comparison
///
struct CSVComparison
{
    std::string name;
    double percrentDifference = 2.0;
    bool passed = true;
    CSVSummary computedSummary;
};

///
/// \brief Calculate the error between two CSV collections
/// \param a CSV data container to compare
/// \param a CSV data container to compare
/// \param statistical results of the comparison
/// \return False if any error occured during processing (ex. unequal amount of data)
///
bool compareCSVs(const CSV& baseline, const CSV& computed, CSVComparison& cmp);

///
/// \brief Calculate the error between two CSV files
/// \param a CSV filename to compare
/// \param a CSV filename to compare
/// \param statistical results of the comparison
/// \return False if any error occured during processing (ex. unequal amount of data)
///
bool compareCSVs(std::string const& baseline, std::string const& computed, CSVComparison& cmp);

///
/// \brief Compare two CSV files and create a line plot containing the 2 vectors associated with a header
/// \param a CSV filename to compare
/// \param a CSV filename to compare
/// \param statistical results of the comparison
/// \param directory to place plot files
/// \param option to not create plots if no errors exceed the provided percent difference
/// \return False if any error occured during processing (ex. unequal amount of data)
///
bool compareAndPlotTimeSeriesCSVs(std::string const& baseline, std::string const& computed, CSVComparison& cmp, std::string const& outDir, bool onlyPlotOnError = true);

///
/// \struct PointPlotConfig
///
/// \brief Contains several options to use creating a plot
///
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

///
/// \brief Calculate the percent difference between two values (difference / average)
///
double percentDifference(double expected, double calculated, double epsilon = 1E-20);
///
/// \brief Calculate the percent tolerance between two values (difference / expected)
///
double percentTolerance(double expected, double calculated, double epsilon  = 1E-20);

///
/// \struct CSVPlotSource
///
/// \brief Contains the data and its drawing options to use when plotted, such as line color and pen type
///
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

///
/// \brief Create line plot for each header in CSVPlotSource.
///        Each plot will have a line from each CSVPlotSource of the same named header.
///        PlotConfig options are hardcoded
/// \param Base file path name, each plot file name will start with this
/// \param vector of data to plot
/// \param flag to compute a ymax and ymin based on all plot sources
/// \return False if any error occured during processing (ex. unequal amount of data)
///
bool plotTimeSeriesCSV(std::string const& base_name, std::vector<CSVPlotSource> const& srcs, bool const_y_axis=false);

///
/// \brief Create line plot for each header in CSVPlotSource.
///        Each plot will have a line from each CSVPlotSource of the same named header.
/// \param Base file path name, each plot file name will start with this
/// \param vector of data to plot
/// \param plot configuration options to use
/// \return False if any error occured during processing (ex. unequal amount of data)
///
bool pointPlot2DCSVs(std::string const& base_name, std::vector<CSVPlotSource> const& srcs, PointPlotConfig& cfg);
} // namespace imstk
