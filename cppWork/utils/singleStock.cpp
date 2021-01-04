/**
 * What I cant do:
 * average Directional Index -> formula requires itself already????
 * Balance of power -> we don't have opening prices
 * DEMA
 * Hull moving avg
 * tripple exponential moving avg.
 * Volume-by-Price  --> This on is confusing? 12 equal price zones? What does that mean? and Positive and negative volumes, what?
 * Accumulation/ Distribution line -> To get a point you need the points from every day in the past. We can deal with this when we are using it in the ML software. 
 * The Chande Trend Meter (CTM) doesn't have a formula online that I could find
 * Price Relative / Relative Strength -> this is used to just compare two stocks, we can do this on our own using the DB i think, not to mention it's highly customizable so we could make a bunch for this even, check it out https://school.stockcharts.com/doku.php?id=technical_indicators:price_relative
 * RRG Relative Strength -> No formula for this
 * 
 */


using namespace std;

#include "EnumsAndFunctions.h"
#include <ctype.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <cmath>

class singleStock {
public:
    
    data obj;

    /** This opens a file that's in the same format as trader_System outputs (csv) and puts all the data in arrays
     * input the file name as a string 
     * Output: File length on success, -1 on failure.
     */
    int getStockHistory(string filename) {
        fstream file;

        file.open(filename, ios::in);
        if (!file.is_open()) {
            cout << "Failed to open file" << endl;
            return -1;
        }

        obj.length = getNumberOfLines(&file) - 1;
        file.clear();
        file.seekg(0, ios::beg);

        obj.row = new basicData[obj.length];

        //Jump to the start of the file.
        string line;
        getline(file, line);    //Skip the first line
        
        //Get all the tickers and put them in the object.
        int counter = 0;
        while (getline(file, line)) {
            int comma;
            string text;
            
            //Get the data
            comma = line.find(',');
            text = line.substr(0, comma);
            obj.row[counter].date = text;
            line = line.substr(comma+1);

            //Get the high
            comma = line.find(',');
            text = line.substr(0, comma);
            obj.row[counter].high = atof(text.c_str());
            line = line.substr(comma+1);

            //Get the low
            comma = line.find(',');
            text = line.substr(0, comma);
            obj.row[counter].low = atof(text.c_str());
            line = line.substr(comma+1);

            //Get the close
            comma = line.find(',');
            text = line.substr(0, comma);
            obj.row[counter].close = atof(text.c_str());
            
            line = line.substr(comma+1);

            //Get the adj close
            comma = line.find(',');
            text = line.substr(0, comma);
            obj.row[counter].adjustedClose= atof(text.c_str());
            line = line.substr(comma+1);

            //Get the volume
            obj.row[counter].volume = atoi(line.c_str());

            counter++;
        }

        file.close();
        return obj.length;
    }

    /** returns the number of lines in a file.
     * input the file (that has been confirmed open)
     * outputs the number of lines in the file
     */
    int getNumberOfLines(fstream *file) {
        int count = 0;
        string line;

        file->seekg(0, ios::beg);
            
        while (getline(*file, line)) {
            count++;
        }

        return count;
    }

    /**
     * Checks the inputs so I don't have to keep checking them
     * input the start and end times
     * output: false on failure, true on success
     */
    bool checkInputs(int start, int end) {
        if (start > obj.length) return false;
        if (start <= end) return false;
        if (end < 0) return false;
        return true;
    }

    //For testing
    void printVals(int start = 0, int end = 0) {
        cout << "printing vals in range" << endl;

        for (int i = -1; i < 5; i++) {
            cout << obj.row[i].close << ", ";
        
        }

        cout << endl;
    }

    /**
     * Finds the weighted moving avg between two times, reference: https://corporatefinanceinstitute.com/resources/knowledge/trading-investing/weighted-moving-average-wma/#:~:text=Summary-,The%20weighted%20moving%20average%20(WMA)%20is%20a%20technical%20indicator%20that,summing%20up%20the%20resulting%20values.
     * input: The start and end times
     * output: -1 on failure, the weighted moving avg on success
     */
    double weightedMovingAvg(int start, int end) {
        if (!checkInputs(start, end)) return -1;

        int weight = 0;
        for (int i = 0; i <= (start - end); i++) {
            weight += i;
        }

        double sum = 0;

        int counter = 1;
        for (int i = (obj.length) - start; i < obj.length - end; i++) {
            sum += obj.row[i].close * (counter * weight);

            counter++;
        }

        return sum;
    }

       
    /**
     * Find the moving avg. for any specific time period
     * Input: the start date-> how many days back you want to start. end-> how many days back you want it to end. Ex: If you want the last twenty days do start = 20, end = 0;
     * output: the Avg as a double, -1 on failure.
     */
    double simpleMovingAvg(int start, int end) {
        if (!checkInputs(start, end)) return -1;
        double sum = 0;
        
        for (int i = (obj.length) - start; i < obj.length - end; i++) {
            sum += obj.row[i].close;
        }

        return sum/(start - end);
    }

    /**
     * Find the ema for any specific time period
     * input:  the start date-> how many days back you want to start. endDate-> how many days back you want it to end. Ex: If you want the last twenty days do start = 20, end = 0;
     * output: the ema for the that range, or -1 for failure
     */
    double exponentialMovingAvg(int startDate, int endDate, int smoothingFactor = 2) {
        if (!checkInputs(startDate, endDate)) return -1;
        if (smoothingFactor <= 0) return -1;
        
        int numDays = startDate - endDate;
        double weight = smoothingFactor / (numDays + 1);

        double ema = (obj.row[ (obj.length-1) - (endDate) ].close * weight);
        if (numDays != 0) {
            ema += exponentialMovingAvg(startDate, endDate+1) * (1 - weight);
        }
        return ema;
    }

    /**
     * Find the ema FOR VOLUME for any specific time period
     * input:  the start date-> how many days back you want to start. endDate-> how many days back you want it to end. Ex: If you want the last twenty days do start = 20, end = 0;
     * output: the ema for the that range, or -1 for failure
     */
    double exponentialMovingAvgVolume(int startDate, int endDate, int smoothingFactor = 2) {
        if (!checkInputs(startDate, endDate)) return -1;
        if (smoothingFactor <= 0) return -1;
        
        int numDays = startDate - endDate;
        double weight = smoothingFactor / (numDays + 1);

        double ema = (obj.row[ (obj.length-1) - (endDate) ].volume * weight);
        if (numDays != 0) {
            ema += exponentialMovingAvgVolume(startDate, endDate+1) * (1 - weight);
        }
        return ema;
    }

    /**
     * returns the Moving Average Convergence Divergence (MACD). Reference https://www.investopedia.com/terms/m/macd.asp
     * input (optional): Gives the macd for INPUT days in the past. So for today it's 0, two weeks ago its 10 (5 trading days per week)
     * output: The double value for the MACD, or -1 for failure
     */
    double macd(int time) {
        double minuend = exponentialMovingAvg(12 + time, time);
        double subtrahend = exponentialMovingAvg(26 + time, time);
        
        if (minuend == -1 || subtrahend == -1) return -1;

        return minuend - subtrahend;
    }

    /**
     * Finds the mean stock closing price between two dates.
     * input: the start and end date
     * output the mean, -1 on failure (improper inputs)
     */
    double getMean(int start, int end) {
        if (!checkInputs(start, end)) return -1;

        double mean;

        for (int i = (obj.length) - start; i < obj.length - end; i++) {
            mean += obj.row[i].close;
        }

        return mean/(start - end);
    }

    /**
     * Find the varience of the stock CLOSING PRICES between two dates.
     * input: The start and end as integers that are based on the current day. So if you wanted the last twenty days you'd do start=20, end= 0
     * output: The varience, -1 on failure
     */
    double varience(int start, int end) {
        if (!checkInputs(start, end)) return -1;
        double mean, numorator = 0;

        mean = getMean(start, end);

        for (int i = (obj.length) - start; i < obj.length - end; i++) {
            numorator += pow( (obj.row[i].close - mean), 2);
        }

        return ( numorator / (start - end) );
    }
    
    /**
     * Find the standard Deviation of the CLOSING PRICES of a stock between two dates
     * input: The start and end as integers that are based on the current day. So if you wanted the last twenty days you'd do start=20, end= 0
     * output: the SD as a double or -1 on failure
     */
    double standardDeviation(int start, int end) {
        if (!checkInputs(start, end)) return -1;

        return sqrt( varience(start, end) );
    }

    /**
     * Gets the upper bollinger band at a point ending at input n for data starting at input start
     * input: the start and end of the data being used as integers, OPTIONAL: the number of standard deviations you want to use and the smoothing factor
     * output: -1 on failure, the upper bollinger band point on success
     */
    double bollingerBandUpper(int start, int end, int numStandardDeviations = 2, int smoothingFactor = 20) {
        if (!checkInputs(start, end)) return -1;
        if (numStandardDeviations < 0) return -1;
        if (smoothingFactor <= 0) return -1;

        double ma = exponentialMovingAvg(start, end, smoothingFactor);
        double sd = standardDeviation(start, end);

        if (ma == -1 || sd == -1) return -1;

        return ma + ( numStandardDeviations * sd);
    }

    /**
     * Gets the lower bollinger band at a point ending at input n for data starting at input start
     * input: the start and end of the data being used as integers, OPTIONAL: the number of standard deviations you want to use and the smoothing factor
     * output: -1 on failure, the lower bollinger band point on success
     */
    double bollingerBandLower(int start, int end, int numStandardDeviations = 2, int smoothingFactor = 20) {
        if (!checkInputs(start, end)) return -1;
        if (numStandardDeviations < 0) return -1;
        if (smoothingFactor <= 0) return -1;

        double ma = exponentialMovingAvg(start, end, smoothingFactor);
        double sd = standardDeviation(start, end);

        if (ma == -1 || sd == -1) return -1;

        return ma - ( numStandardDeviations * sd);
    }

    /**
     * The middle band is a simple moving average that is usually set at 20 periods
     * input: the end date, and the range
     * output: -1 on failure, a double on success
     */
    double bollingerBandMiddle(int end, int range = 20) {
        return simpleMovingAvg(end + range, end);
    }

    /**
     * Find the avg upward price change
     * input the start and end times
     * output -1 on failure, the avg positive change on success
     */
    double avgUpwardPrice(int start, int end) {
        if (!checkInputs(start + 1, end)) return -1;

        double change = 0;
        double count = 0;
        
        for (int i = (obj.length) - start + 1; i < obj.length - end; i++) {
            double current = obj.row[i].close;
            double previous = obj.row[i - 1].close;

            if (current - previous > 0) {
                change += current - previous;
                count++;
            }
        }

        if (change == 0) return change;

        return change/count;
    }

    /**
     * Find the avg downward price change
     * input the start and end times
     * output -1 on failure, the avg neative change on success
     */
    double avgDownwardPrice(int start, int end) {
        if (!checkInputs(start, end)) return -1;
        
        double change = 0;
        double count = 0;
        
        for (int i = (obj.length) - start; i < obj.length - end; i++) {
            double current = obj.row[i].close;
            double previous = obj.row[i-1].close;

            if (current - previous < 0) {
                change += current - previous;
                count++;
            }
        }

        if (change == 0) return change;

        return change/count;
    }

    /**
     * Find the Relative Strength Index (RSI) ending at a endpoint
     * input: the start point that is the start of the data given , endpoint is the point that is going to be given
     * output: -1 on failure, the double rsi on success
     */
    double rsi(int start,int end) {
        if (!checkInputs(start, end)) return -1;

        double up = avgUpwardPrice(start, end);
        double down = avgDownwardPrice(start, end);

        if (up == 0 || down == 0) return 100;

        return 100 - ( 100/ ( 1 + ( up / down)));
    }

    //Helper function for the fibbanacci function. 
    string fibHelp(double curr, double diff, double per) {
        return to_string(curr - (diff * per));
    }

    /**
     * Gives the fibonacci lines for a stock price jump as a JSON string
     * input the previous stock price and current stock price as doubles
     * output: all the prices and their labels as a JSON string
     */
    string fibonacci(double previous, double current) {
        if (previous < 0 || current < 0) return "-1";

        double difference = current - previous;

        //Assumong the most common ratios of: 23.6, 38.2, 50 ,61.8, 78.6 percents
        string returnVal = "{\"23.6\":\"";
        returnVal += fibHelp(current, difference, 1.236);
        returnVal += "\",\"38.2\":\"";
        returnVal +=  fibHelp(current, difference, 1.382);
        returnVal += "\",\"50\":\"";
        returnVal +=  fibHelp(current, difference, 1.5);
        returnVal += "\",\"61.8\":\"";
        returnVal +=  fibHelp(current, difference, 1.618);
        returnVal += "\",\"78.6\":\"";
        returnVal +=  fibHelp(current, difference, 1.786);
        returnVal += "\"}";

        return returnVal;
    }

    /**
     * Find the highest stock price in a given period
     * input: the start and end times
     * output: The highest stock price in the period, -1 on failure
     */
    double periodHigh(int start, int end) {
        if (!checkInputs(start, end)) return -1;
        double high = -1;

        for (int i = (obj.length) - start; i < obj.length - end; i++) {
            if (obj.row[i].close > high) high = obj.row[i].close; 
        }

        return high;
    }

    /**
     * Find the lowest stock price in a given period
     * input: the start and end times
     * output: The lowest stock price in the period, -1 on failure
     */
    double periodLow(int start, int end) {
        if (!checkInputs(start, end)) return -1;

        double low;
        if (start == obj.length) {
            low = obj.row[ (obj.length) - start ].close;
        } else {
            low = obj.row[ (obj.length - 1) - start ].close;
        }
        

        //start a element after start as that value is already in lowest
        for (int i = (obj.length) - start; i < obj.length - end; i++) {
            if (obj.row[i].close < low) low = obj.row[i].close; 
        }

        return low;
    }

    /**
     * Find the Inchimoku Cloud Span A, returns a single point on the span ending at input end.
     * input: The end point how want to find the span A point for at that same time
     * output: The Senjukou or Leading Span A value on success, - 1 on failure 
     */
    double inchimokuCloudA(int end) {
        if (end < 0) return -1;


        double conversionLine = (periodHigh(end + 9, end) + periodLow(end + 9, end)) / 2;   //tenkan sen
        double baseLine = (periodHigh(end + 26, end) + periodLow(end + 26, end)) / 2;      //kijun sen

        if (conversionLine == -1 || baseLine == -1) return -1;   

        return (conversionLine + baseLine) / 2; //senjkou span A
    }

    /**
     * Find the Inchimoku Cloud Span B, returns a single point on the span ending at input end.
     * input: The end point how want to find the Span B point for at that same time
     * output: The Senjukou or Leading Span B value on success, - 1 on failure 
     */
    double inchimokuCloudB(int end) {
        if (end < 0) return -1;

        double high = periodHigh(end + 52, end);
        double low = periodLow(end + 52, end);

        if (high == -1 || low == -1) return -1;

        return  (high + low) / 2;  //senjkou span B
    }

    /**
     * Gets the anchored VWAP, reference https://help.optuma.com/kb/faq.php?id=941#:~:text=The%20simple%20average%20price%20over,after%20Day%205%20of%20%2453.70.
     * input: the start and end that you want this calculated from
     * output: -1 on failure, anchored VWAP on success
     */
    double anchoredVWAP(int start, int end) {
        if (!checkInputs(start, end)) return -1;

        double valueAVG= 0;
        double volumeAVG = 0;

        for (int i = (obj.length) - start; i < obj.length - end; i++) {
            
            valueAVG += obj.row[i].close * obj.row[i].volume;
            volumeAVG += obj.row[i].volume;
        }

        valueAVG /= start-end;
        volumeAVG /= start-end;
        
        return valueAVG/volumeAVG;
    }

    double max(double elems[], int length) {
        if (length == 0) return -1;

        double highest = elems[0];

        for (int i = 1; i < length; i++) {
            if (elems[i] > highest)  highest = elems[i];
        }

        return highest;
    }

    /**
     * Finds the average true range for a function. reference: https://school.stockcharts.com/doku.php?id=technical_indicators:average_true_range_atr
     * input: the end date you want to finish on, and how large of a range you want the avg true range to encompass
     * output: -1 on failure, the double atr on success
     */
    double atr(int end, int range) {
        if (!checkInputs(end + range, end)) return -1;

        double colA[range];
        double colB[range];
        double colC[range];
        double tr[range];

        //First row is unique
        colA[0] = obj.row[end + range].high - obj.row[end + range].low;
        colB[0] = 0;
        colC[0] = 0;
        
        double sum = colA[0];

        int counter = 1;
        for (int i = (obj.length) - end + range + 1; i < obj.length - end; i++) {
            colA[counter] = obj.row[i].high - obj.row[i].low;
            colB[counter] = obj.row[i].high - obj.row[i- 1].close;
            colC[counter] = obj.row[i].low - obj.row[i- 1].close;

            double vals[3] = { colA[counter], colB[counter], colC[counter] };

            sum += max( vals, 3 );

            counter++;
        }

        return sum/range;
    }

    /**
     * Gives the value for the chandelier exit long, reference https://corporatefinanceinstitute.com/resources/knowledge/trading-investing/chandelier-exit/#:~:text=Chandelier%20Exit%20(CE)%20is%20a,it%20(going%20short)..
     * input: the ending date you want, OPTIONAL: the range and the multiple 
     * output: -1 on failure, the double chandelier exit long on success
     */
    double chandelierExitLong(int end, int range = 22, double multiple = 3.0) {
        if (!checkInputs(end + range, end)) return -1;
        double highestHigh = periodHigh(end + range, end);

        double theAtr = atr(end, range);

        if (theAtr == -1) return -1;

        return highestHigh - (theAtr * multiple);
    }

    /**
     * Gives the value for the chandelier exit short, reference https://corporatefinanceinstitute.com/resources/knowledge/trading-investing/chandelier-exit/#:~:text=Chandelier%20Exit%20(CE)%20is%20a,it%20(going%20short)..
     * input: the ending date you want, OPTIONAL: the range and the multiple 
     * output: -1 on failure, the double chandelier exit short on success
     */
    double chandelierExitShort(int end, int range = 22, double multiple = 3.0) {
        if (!checkInputs(end + range, end)) return -1;
        double highestHigh = periodHigh(end + range, end);

        double theAtr = atr(end, range);

        if (theAtr == -1) return -1;

        return highestHigh + (theAtr * multiple);
    }

    /**
     * Efficiency Ratio (ER)
     * Input: the end value you want this calculated for, OPTIONAL: the number of periods you want this calculated for
     * Ouput: -1 on failure, double ER on success. 
     */
    double er(int end, int numberPeriods = 10) {
        if (!checkInputs(end + numberPeriods, end)) return -1;
        int length = obj.length;

        double change = abs( obj.row[length - end].close - obj.row[length - (end + numberPeriods)].close);

        double volatilitySum = 0;

        for (int i = length - (end + numberPeriods); i < length - end - 1; i++) {
            volatilitySum += abs( obj.row[i].close - obj.row[i + 1].close);
        }

        return change/volatilitySum;
    }

    /**
     * returns the smoothing constant, used in finding the KAMA, reference: https://school.stockcharts.com/doku.php?id=technical_indicators:kaufman_s_adaptive_moving_average
     * input: the end point this is based on. OPTIONAL the number of periods, slowSC and FastSC
     * output: -1 on failure, a double on success
     */
    double sc(int end, int numPerdiods = 10, int slowSC = 30, int fastSC = 2) {

        double step = ( 2 / ( fastSC + 1 ) ) - ( 2 / ( slowSC + 1 ));
        step *= er(end, numPerdiods);
        step += 2 / ( slowSC + 1);

        return pow( step, 2 );
    }

    /**
     * Finds the KAMA as explained in https://school.stockcharts.com/doku.php?id=technical_indicators:kaufman_s_adaptive_moving_average
     * input: the input is the end day, OPTIONAL: the start, slowSC and fast SC values
     * oupit: -1 on failure, a double on success
     */
    double kama(int end, int start = -1, int slowSC = 30, int fastSC = 2) {
        double prior, theSC;
        
        if (start == -1) {
            if (!checkInputs(start, end)) return -1;

             //Pror KAMA is the simple moving avg according to https://school.stockcharts.com/doku.php?id=technical_indicators:kaufman_s_adaptive_moving_average
            prior = simpleMovingAvg(end + 10, end);
            theSC = sc(end);
        } else {
            //Manual input check here
            if (end < 0) {
                return -1;
            } else if (end + 10 > obj.length) {
                return -1;
            }

            prior = simpleMovingAvg(start, end);
            theSC = sc(end, start - end);
        }

        if (prior == -1 || theSC == -1) return -1;

        return prior + ( theSC * (obj.row[ (obj.length) - end].close - prior));
    }

    /** One third of the Kelter Channels
     * input the start and end of the data
     * output: -1 on failure, double value on success
     */
    double kelterChannelUpper(int start, int end) {
        if (!checkInputs(start, end)) return -1;
        
        double ema = exponentialMovingAvg(start, end);
        double theATR = atr(end, start - end);

        if (ema == -1 || theATR == -1) return -1;

        return ema + ( 2 * theATR);
    }

    /** One third of the Kelter Channels
     * input the start and end of the data
     * output: -1 on failure, double value on success
     */
    double kelterChannelMiddle(int start, int end) {
        return exponentialMovingAvg(start, end);
    }

    /** One third of the Kelter Channels
     * input the start and end of the data
     * output: -1 on failure, double value on success
     */
    double kelterChannelLower(int start, int end) {
        if (!checkInputs(start, end)) return -1;
        
        double ema = exponentialMovingAvg(start, end);
        double theATR = atr(end, start - end);

        if (ema == -1 || theATR == -1) return -1;

        return ema - ( 2 * theATR);
    }

    /**
     * Returns the pivot point on the day selected. The pivot point is the avg of the high, low, and close price
     * input: The day you want the pivot point for
     * output: -1 on invalid input day, the double pivot point for success.
     */ 
    double pivotPoints(int day) {
        if (day > obj.length || day < 0) return -1;

        int index = obj.length - day;

        return ( obj.row[index].high + obj.row[index].close + obj.row[index].low ) / 3;
    }

    /**
     * Finds the upper price channel for the day given a look back period
     * input: the day you want it for, OPTIONAL: the look back period length
     * Output: -1 on failure, a double on success
     */
    double upperPriceChannel(int day, int lookBackPeriod = 20) { //Assuming 20 days previous look back as specified in https://school.stockcharts.com/doku.php?id=technical_indicators:price_channels
        if (day > obj.length || day < 0) return -1;

        int lookBack = lookBackPeriod;
        if (obj.length - day < lookBackPeriod) lookBack = obj.length - day;

        double high = 0;
        for (int i =  obj.length - day; i < lookBack + obj.length; i++) {
            if (obj.row[i].close > high) high = obj.row[i].close;
        }

        return high;
    }

    /**
     * Finds the low price channel for the day given a look back period
     * input: the day you want it for, OPTIONAL: the look back period length
     * Output: -1 on failure, a double on success
     */
    double lowerPriceChannel(int day, int lookBackPeriod = 20) { //Assuming 20 days previous look back as specified in https://school.stockcharts.com/doku.php?id=technical_indicators:price_channels
        if (day > obj.length || day < 0) return -1;

        int lookBack = lookBackPeriod;
        if (obj.length - day < lookBackPeriod) lookBack = obj.length - day;

        double low = obj.row[ obj.length - day ].close;
        for (int i =  obj.length - day + 1; i < lookBack + obj.length; i++) {
            if (obj.row[i].close < low) low = obj.row[i].close;
        }

        return low;
    }


    /**
     * Finds the VWAP between to specified dates
     * input: the start and end dates
     * output: -1 on failure, a double on success.
     */
    double VWAP(int start, int end) {
        if (!checkInputs(start, end)) return -1;

        double numoratorSum = 0;
        double denominatorSum = 0;

        for (int i = obj.length - start; i < obj.length - end; i++) {
            
            numoratorSum += (obj.row[i].close * obj.row[i].volume);
            denominatorSum += obj.row[i].volume;
        }

        return numoratorSum / denominatorSum;
    }

    /**
     * Finds the arroon up a point given a period look back length
     * input: the day you want it for, OPTIONAL: the period length
     * output: -1 on failure, a double on success
     */
    double aroonUp(int day, int period = 25) {
        if (day > obj.length || day < 0) return -1;

        int length = period;
        if (obj.length - day < period) length = obj.length - day;

        double high = 0;
        int timeSinceHigh = 0;

        for (int i = obj.length - day; i < obj.length - length; i++) {
            if (high < obj.row[i].close) {
                timeSinceHigh = 0;
                high = obj.row[i].close;

            } else {
                timeSinceHigh++;
            }
        }

        return (( length - timeSinceHigh) / length ) * 100;
    }

    /**
     * Finds the arroon down a point given a period look back length
     * input: the day you want it for, OPTIONAL: the period length
     * output: -1 on failure, a double on success
     */
    double aroonDown(int day, int period = 25) {
        if (day > obj.length || day < 0) return -1;

        int length = period;
        if (obj.length - day < period) length = obj.length - day;

        double low = obj.row[obj.length - day].close;
        int timeSinceLow = 0;

        for (int i = obj.length - (day + 1); i < obj.length - length; i++) { //Starts a day forward 
            if (low < obj.row[i].close) {
                timeSinceLow = 0;
                low = obj.row[i].close;

            } else {
                timeSinceLow++;
            }
        }

        return (( length - timeSinceLow) / length ) * 100;
    }

    /**
     * Finds the aroon Oscillator
     * input: the day, OPTIONAL: the period look back length
     * output: -1 on failure, a double on success
     */
    double aroonOscillator(int day, int period= 25) {
        if (day > obj.length || day < 0) return -1;

        return ( aroonUp(day, period) - aroonDown(day, period));
    }

    //assume 'Current High/Low' means the daily high/low
    /**
     * Find the true range for a point
     * input: the day you want the true range for
     * output: -1 on failure, a double on success
     */
    double trueRange(int day) {
        if (day < 0 || obj.length - (day -1 ) < 0) return -1;
        
        double arr[3];
        int time = obj.length - day;

        arr[0] = obj.row[time].high - obj.row[time].low;
        arr[1] = obj.row[time].high - obj.row[time - 1].close;
        arr[2] = obj.row[time].low - obj.row[time - 1].close;

        return max(arr, 3);
    }

    /**
     * Finds the average true range for a range
     * input: the end date, OPTIONAL: the range you want for the avg true range
     * output: -1 on failure, a double on success
     */
    double averageTrueRange(int end, int range = 14) {
        if (!checkInputs(end + range, end)) return -1;

        double sum = 0;

        for (int i = end; i < end + range; i++) {
            sum += trueRange(i);
        }

        return sum / range;
    }

    //assume 'current high' == daily high
    /**
     * finds the positiveDirectionalMovement which is used in the ADX
     * input: the end date and the period look back length
     * output: a double on success, -1 on failure
     */
    double positiveDirectionalMovement(int end, int periodLength) {
        if (!checkInputs(end + periodLength, end)) return -1;

        double high = 0;

        for (int i = obj.length - end; i < obj.length - (end + periodLength); i++) {
            if (obj.row[i].high > high) high = obj.row[i].high; 
        }

        return obj.row[0].high - high;
    }


    //assume 'current high' == daily high
    /**
     * finds the negativeDirectionalMovement which is used in the ADX
     * input: the end date and the period look back length
     * output: a double on success, -1 on failure
     */
    double negativeDirectionalMovement(int end, int periodLength) {
        if (!checkInputs(end + periodLength, end)) return -1;

        double low = obj.row[ obj.length - end ].low;

        for (int i = obj.length - end + 1; i < obj.length - (end + periodLength); i++) {
            if (obj.row[i].low > low) low = obj.row[i].low; 
        }

        return obj.row[0].low - low;
    }

    /**
     * Finds the smoothed value for calculating the ADX
     * input: the end date, the range, and a string containging either positive or negative 
     * output: -1 on invalid inputs, a double on success
     */
    double smoothed(int end, int range, string posORneg) {
        if (!checkInputs(end + range, end)) return -1;

        int i = 0;
        char c;
        double returnVal = 0;

        while ( posORneg[i]) {
            c=posORneg[i];
            putchar (tolower(c));
            i++;
        }


        if ( posORneg.compare("negative") == 0) {
            double a = 0;
            double b = 0;
            double cdm;

            for (i = 0; i < 14; i++) {
                double dm = negativeDirectionalMovement(end, range);

                if (i == 0) {
                    cdm = dm;
                }

                a += dm;
                b += dm;
            }

            b / 14;
            
            return a - b + cdm;

        } else if ( posORneg.compare("positive") == 0) {    //Did these larger if blocks so it wouldn't have to check positive or negative each time. This will make it faster but the code longer which is an ok tradeoff

            double a = 0;
            double b = 0;
            double cdm;

            for (i = 0; i < 14; i++) {
                double dm = positiveDirectionalMovement(end, range);

                if (i == 0) {
                    cdm = dm;
                }

                a += dm;
                b += dm;
            }

            b / 14;

            return a - b + cdm;
        } else {
            return -1;
        }
    }

    //The formula is kinda weird
    //double averageDirectionalIndex() {
    //}

    /**
     * Finds the bollingerBandWidth 
     * input: start and end value, OPTIONAL: the number of standard deviations, and the smoothing factor
     * output: -1 on failure, a double on success
     */
    double bollingerBandWidth (int start, int end, int numStandardDeviations = 2, int smoothingFactor = 20) {
        if (!checkInputs(start, end)) return -1;
        if (numStandardDeviations < 0) return -1;
        if (smoothingFactor <= 0) return -1;
        
        double upper, lower, middle;

        upper = bollingerBandUpper(start, end, numStandardDeviations, smoothingFactor);
        lower = bollingerBandLower(start, end, numStandardDeviations, smoothingFactor);
        middle = bollingerBandMiddle(end, start - end);

        return ( ( upper - lower) / middle ) * 100; 
    }

    /**
     * Finds the %B for a given price
     * input: the start and end times, OPTIONAL: the number of standard deviations and the smoothing factor
     * output: -1 on failure/ inalid inputs, a double on success
     */
    double percentB (int start, int end, int numStandardDeviations = 2, int smoothingFactor = 20) {
        if (!checkInputs(start, end)) return -1;
        if (numStandardDeviations < 0) return -1;
        if (smoothingFactor <= 0) return -1;
        
        double upper, lower;

        upper = bollingerBandUpper(start, end, numStandardDeviations, smoothingFactor);
        lower = bollingerBandLower(start, end, numStandardDeviations, smoothingFactor);

        return ( obj.row[ obj.length - end].close - lower) / ( upper - lower);
    }

    /**
     * Helper function for the chaikin money flow
     * input: the end date u want
     * output: -1 on failure, a double on success
     */
    double moneyFlowVolume(int end) {
        if ( end < 0  || obj.length - end < 0) return -1;

        int time = obj.length - end;

        //Money Flow Multiplier
        double val = ( ( obj.row[time].close - obj.row[time].low) - ( obj.row[time].high - obj.row[time].close) ) / ( obj.row[time].high - obj.row[time].low);

        //Money Flow Volume
        return val * obj.row[time].volume;
    }

    /**
     * Finds the chaikin money flow
     * input: the end date, OPTIONAL: the period length
     * output: -1 on failure, a double on success
     */
    double chaikinMoneyFlow(int end, int period = 20) {
        if ( end < 0  || obj.length - end < 0) return -1;

        double numorator = 0;
        double denominator = 0;

        for (int i = end; i < end + period; i++) {
            numorator = moneyFlowVolume(i);
            denominator = obj.row[ obj.length - i].volume;
        }

        return numorator/denominator;
    }

    /**
     * The typical price formula. From https://www.investopedia.com/terms/c/commoditychannelindex.asp that is a helper for the Commodity Channel Index
     * input: the start and end dates ****THIS DOES NOT CHECK THE INPUTS SO IT CAN CRASH
     * Output: A double
     */
    double typicalPrice(int start, int end) {
        //if (!checkInputs(start, end)) return -1;

        double sum = 0;

        for (int i = obj.length - start; i < obj.length - end; i++) {
            sum += (obj.row[i].high + obj.row[i].close + obj.row[i].low) / 3;
        }

        return sum;
    }
    
    /**
     * Finds the Commodity Channel Index as seen at https://www.investopedia.com/terms/c/commoditychannelindex.asp
     * Input: The start and end dates
     * Output: -1 on failure, a double on success
     */
    double commodityChannelIndex(int start, int end) {
        if (!checkInputs(start, end)) return -1;

        double tp = 0;  //Typical Price
        double md = 0;  //Mean Deviation
        double ma = 0;  //Moving avg

        tp = typicalPrice(start, end);

        ma = tp / (start - end);
        md = abs( tp - ma) / (start - end);

        return (tp - ma) / (0.015 * md);
    }

    /**
     * This counts the number of days that have a streak that is either up or down in a row. If it's up or down is determined by if the start day is an up or down day
     * Input: the start day
     * Ouput: a positive double for number of down days in a row, or a negative double for number of down days in a row
     */
    double upDownStreak(int start) {    //Counts backwards from the start date
        bool isPos;

        int time = obj.length - start;
        isPos = obj.row[time].close - obj.row[time - 1].close > 0;

        double streak = 1;
        time += 1;
        while (time > 0) {

            if ( isPos == (obj.row[time].close - obj.row[time - 1].close > 0)) {
                streak++;
            
            } else {
                break;
            }

            time -= 1;
        }

        if (isPos == false) return streak * -1;

        return streak;
    }

    /** NOTE: this does not have input checking
     * Finds the number of days that have a lower daily change than the given price change
     * input: the start and end dates, and a double that is the price change that everything is going to be coompared against
     * output: A double
     */ 
    double magnitudeOfPriceChange(int start, int end, double priceChange) {
        double count = 0;
        double length = (double)start - (double)end;

        for (int i = obj.length - start; i < obj.length - end; i++) {
            if (obj.row[i].close - obj.row[i - 1].close < priceChange) {
                count++;
            }
        }

        return count/length;
    }


    /**
     * Finds Connors RSI as described on https://school.stockcharts.com/doku.php?id=technical_indicators:connorsrsi
     * Input: the start and dates
     * Output: A double or -1 on failure
     */
    double connorsRSI(int start, int end) {
        if (!checkInputs(start, end)) return -1;

        double theRSI = rsi(start, end);
        double streak = upDownStreak(start);
        double magnitude = magnitudeOfPriceChange(start, end, obj.row[obj.length - start].close - obj.row[obj.length - start - 1].close);

        return ( theRSI + streak + magnitude) / 3;
    }

    /** NOTE: this does not have input error checking
     * Finds the eleven day Rate of change for a given end point, so it looks 14 days in the past
     * input: the end day that you want the ROC for
     * output: a double
     */
    double elevenDayROC(int end) {
        return ( (obj.row[end].close - obj.row[end - 11].close) - 1 ) * 100;
    }

    /** NOTE: this does not have input error checking
     * Finds the fourteen day Rate of change for a given end point, so it looks 14 days in the past
     * input: the end day that you want the ROC for
     * output: a double
     */
    double fourteenDayROC(int end) {
        return ( (obj.row[end].close - obj.row[end - 14].close) - 1 ) * 100;
    }

    /** NOTE: This does not have error checking on the inputs, THIS CAN FAIL
     * Finds the weighted moving avg for a series of points
     * input: an array of points and the length of the array
     * output: a double
     */
    double wma(double inputs[], int length) {
        double wma = 0;


        for (int i = 0; i < length; i++) {
            wma += inputs[i] * ( (i + 1) / length);
        }

        return wma /  ( ( length * ( length + 1) ) / 2);
    }


    /**
     * Finds a point on the cappock Curve at spot: end
     * input: The end date you want
     * output: -1 on failure, a double on success
     */
    double coppockCurve(int end) {
        if (!checkInputs(end - 32, end)) return -1; //Furthest back being checked is 32 days or ~ one month
        int LENGTH = 10;


        double ROCS[LENGTH];
        int count = 0;

        for (int i = obj.length - end - 10; i < obj.length - end; i++) {
            
            ROCS[count] = elevenDayROC(i) + fourteenDayROC(i);
            count++;
        }

        return wma(ROCS, LENGTH);
    }

    /**
     * Finds the correlation Coefficient between a start and end point
     * Input: the start and end points
     * output: -1 on failure, a double on success
     */
    double correlationCoefficient(int start, int end) {
        if (!checkInputs(start, end)) return -1;
        int length = end - start;

        //Find the means
        double xMean = 0;
        double yMean = 0; 

        for (int i = obj.length - end; i < obj.length - start; i++) {
            
            xMean += i;
            yMean += obj.row[i].close;
        }

        xMean /= length;
        yMean /= length;

        //Find the standard deviations
        double ySD = standardDeviation(start, end);

        double xSD = 0;

        for (int i = obj.length - end; i < obj.length - start; i++) {
            
            xSD += pow((i - xMean), 2);
        }

        xSD /= ( length - 1);
        xSD = sqrt(xSD);

        //Find the standardized Values then find the Correlation Coefficient
        double xArr, yArr, cc = 0;

        for (int i = 0; i < length; i++) {
            xArr = ((length + end) - xMean) - xSD;
            yArr = (obj.row[obj.length + length + end].close - yMean) - ySD;

            cc += xArr * yArr;
        }

        return cc / length;
    }


    //Not done, I need to come back to this -> Different websites say different things so it can be hard to follow
    double pmoSignalLinePoint(int start, int end) {
        if (!checkInputs(start, end)) return -1;
        int length = end - start;

        //Get the smoothing line
        double smoothing = 2 / length;

        //Finds the 1-bar ROC
        double roc = (obj.row[ obj.length - start].close - obj.row[ obj.length - start + 1].close) / (obj.row[ obj.length - start + 1].close * 100);

        //Finds the nontraditional price change
         
    }

    /**
     * Finds the Detrended Price Oscillator
     * input: the start date (number of days back from current that you want to start on), the lookback length
     * output: -1 on failure, a double on success
     */
    double DPO(int start, int lookBackLength) {
        if (!checkInputs(start, start - lookBackLength)) return -1;

        double minuend, subtrahend;

        minuend = obj.row[obj.length - (start + (lookBackLength / 2) + 1) ].close;
        subtrahend = simpleMovingAvg(start, start + lookBackLength);
        
        if (subtrahend == -1) return subtrahend;

        return minuend = subtrahend;
    }

    /**
     * Finds the Ease Of Movement as seen on https://www.investopedia.com/terms/e/easeofmovement.asp
     * input: the start date and the scale 
     * output: -1 on failure, a double on success
     */
    double emv(int start, int scale) {
        if (start > obj.length || start < 0 ) return -1;
        double distanceMoved, boxRatio;

        distanceMoved = ( ( obj.row[obj.length - start].high + obj.row[obj.length - start].low) / 2 ) - ( ( obj.length));
        
        boxRatio = (obj.row[obj.length - start].volume / scale) / (obj.row[obj.length - start].high - obj.row[obj.length - start].high);

        return distanceMoved / boxRatio;  
    }

    /**
     * Finds the single day forceIndex
     * input: the day
     * output: -1 on failure, a double on success
     */
    double forceIndex(int date) {
        if (date - 1 < 0 || date > obj.length) return -1;

        return (obj.row[obj.length - date].close - obj.row[obj.length - date - 1].close) * obj.row[obj.length - date].volume;

    }


    /**
     * Find the ema for any specific time period
     * input:  the start date-> how many days back you want to start. endDate-> how many days back you want it to end. Ex: If you want the last twenty days do start = 20, end = 0;
     * output: the ema for the that range, or -1 for failure
     */
    double exponentialMovingAvg(double range[], int length, int smoothingFactor = 2) {
        if (smoothingFactor <= 0) return -1;
        
        //double sma = simpleMovingAvg(startDate, endDate);
        double weight = smoothingFactor / (length + 1);

        double ema = (obj.row[ (obj.length-1) - (length) ].close * weight);
        if (length != 0) {
            ema += exponentialMovingAvg(range, length - 1) * (1 - weight);
        }
        return ema;
    }

    /**
     * Finds the force index between two points
     * input: the start and end date
     * output: -1 on failure, a double on success
     */
    double forceIndex(int start, int end) {
        if (!checkInputs(start, end)) return -1;

        int length = end - start;
        double arr[length];
        for (int i = 0; i < length; i++) {
            arr[i] = forceIndex(start + i);

            if (arr[i] == -1) return -1;
        }

        return exponentialMovingAvg(arr, length);
    }


    /**
     * Finds the X period mass index
     * input the end date you're looking for, OPTIONAL: the period length and the subPeriodLookback (look in the function to understand these if you even want to input them)
     * output: -1 on failure, a double on success
     */
    double massIndex(int end, int periodLength = 25, int subPeriodLookBackLength = 9) {
        if (!checkInputs(end + 34, end)) return -1; 
        double emaRatio = 0;

        for (int j = 0; j < periodLength; j++) {

            double singleEMA = exponentialMovingAvg(end + subPeriodLookBackLength + j, end + j);
            
            double arr[subPeriodLookBackLength];
            for (int i = 0; i < subPeriodLookBackLength; i++) {
                arr[i] = obj.row[obj.length + end + i + j].high - obj.row[obj.length + end + i + j].low; 
            }

            double doubleEMA = exponentialMovingAvg(arr, subPeriodLookBackLength);

            emaRatio += singleEMA / doubleEMA;
        }

        return emaRatio;
    }


    /**
     * Finds the money flow index as seen on investopedia.com/terms/m/mfi.asp
     * input: the end date you want it for, OPTIONAL: the period length
     * output: -1 on failure, a double on success
     */
    double moneyFlowIndex(int end, int periodLength = 14) {
        if (!checkInputs(end + periodLength, end)) return -1; 
        
        double pos = 0;
        double neg = 0;
        double currentTP;
        double previousTP = (obj.row[obj.length - end].high + obj.row[obj.length - end].close + obj.row[obj.length - end].low) / 3;

        for (int i = 1; i < periodLength; i++) {
            currentTP = (obj.row[obj.length - end - i].high + obj.row[obj.length - end - i].close + obj.row[obj.length - end - i].low) / 3;

            if (currentTP < previousTP) {//It was down
                neg += currentTP * obj.row[obj.length - end - i].volume * -1;

            } else {    //It was up
                pos += currentTP * obj.row[obj.length - end - i].volume;
            }
        }

        double moneyFlowRatio = pos/neg;

        return 100 - ( 100 / (1 + moneyFlowRatio) );
    }

    /**
     * Recursive function for finding the negativeVolumeIndex
     * Input the end date u want, and the length, OPTIONAL: PNVI doesnt matter
     * output: the NVI 
     */
    double NVI(int date, int length, double PNVI = 1) {
        if (length == 0) return 1;

        double pnvi = NVI(date - 1, PNVI, length - 1);

        return pnvi + ( ( ( obj.row[obj.length + date].close - obj.row[obj.length + date - 1].close ) / obj.row[obj.length + date - 1].close ) * pnvi);
    }

    /**
     * The interface for finding the negative volume index
     * input: the end date you want it for, OPTIONAL: the lookback length
     * output: -1 on failure, a double on success
     */
    double negativeVolumeIndex(int end, int length = 14) {
        if (!checkInputs(end + length, end)) return -1; 
        
        return NVI(end + length, 1, length);
    }


    /**
     * Recurisve function that finds the obv, ASSUME OBV STARTS AT 0
     * input: the day, the look back length, OPTIONAL: the obv amount but this will do nothing
     * output: the obj for the given range as a double
     */
    double obv(int day, int length, double obvAmount = 0) {
        if (length == 0) return 0;

        double amount;

        if (obj.row[obj.length - day].close < obj.row[obj.length - day - 1].close) {
            amount = obj.row[obj.length - day].volume * -1;
        
        } else if (obj.row[obj.length - day].close > obj.row[obj.length - day - 1].close) {
            amount = obj.row[obj.length - day].volume * -1;
        
        } else {
            amount = 0;
        }

        return obv(obvAmount, length - 1, day - 1) + amount;
    }

    /** NOTE: it is possible to have 'failure' as a valid return but the chances are very low
     * user interface for finding the On-Balance Volume
     * input: the end date, OPTIONAL: the look back length
     * output: -1 on failure, a double on success
     */
    double onBalanceVolume(int end, int lookBackLength = 14) {
        if (!checkInputs(end + lookBackLength, end - 1)) return -1;

        return obv(end, lookBackLength); 
    }

    /**
     * Finds the Percentage Price Oscillator as seen on https://www.investopedia.com/terms/p/ppo.asp
     * input: the endDate and the smoothing factor
     * output: -1 on failure, a double on success
     */
    double percentagePriceOscillator(int endDate, int smoothingFactor = 2) {
        double a, b;

        a = exponentialMovingAvg(endDate + 12, endDate, smoothingFactor);
        b = exponentialMovingAvg(endDate + 26, endDate, smoothingFactor);

        if (a == -1 || b == -1) return -1;

        return ((a - b) / b ) * 100;
    }

    /**
     * Finds the Percentage Volume Oscillator as seen on https://www.investopedia.com/terms/p/ppo.asp
     * input: the endDate and the smoothing factor
     * output: -1 on failure, a double on success
     */
    double percentageVolumeOscillator(int endDate, int smoothingFactor = 2) {
        double a, b;

        a = exponentialMovingAvgVolume(endDate + 12, endDate, smoothingFactor);
        b = exponentialMovingAvgVolume(endDate + 26, endDate, smoothingFactor);

        if (a == -1 || b == -1) return -1;

        return ((a - b) / b ) * 100;
    }

    /**  NOTE: there is no error checking in this as any returned number is valid 
     * Finds the momentum for a stock
     * Input: the start and end times
     * Output:a double
     */
    double momentumIndicator(int start, int end) {
        return obj.row[obj.length - start].close - obj.row[obj.length - end].close;
    }


    /**
     * Finds the Williams %R
     * input: the end date your looking at and the lookback length
     * output: -1 on failure, a double on success
     */
    double williamsPercentR(int end, int lookbackLength) {
        if (!checkInputs(end + lookbackLength, end)) return -1;

        double highestHigh = obj.row[obj.length - end].close;
        double lowestLow = obj.row[obj.length - end].close;

        for (int i = end - 1; i > end + lookbackLength; i--) {
            
            if (obj.row[obj.length + i].close < lowestLow) lowestLow = obj.row[obj.length + i].close;

            if (obj.row[obj.length + i].close > highestHigh) highestHigh = obj.row[obj.length + i].close;
        }

        return (highestHigh - obj.row[obj.length - end].close) / (highestHigh - lowestLow);
    }

    /**
     * Finds the rate of change for the volume for the two given days
     * Input: the two days
     * output: -1 on failure, a double on success
     */
    double vroc(int first, int second) {
        if (first < 0 || first > obj.length) return -1;
        if (second < 0 || second > obj.length) return -1;

        return (obj.row[obj.length - second].volume - obj.row[obj.length - first].volume) / (second - first);
    }

    /**
     * Finds the bears power
     * input: the end value and the look back range
     * output: -1 on failure, a double on success
     */ 
    double bearsPower(int end, int range) {
        if (!checkInputs(end + range, end)) return -1;
        double ema = exponentialMovingAvg(end + range, end);

        if (ema == -1) return ema;

        return obj.row[obj.length - end].close - ema; 
    }
};
