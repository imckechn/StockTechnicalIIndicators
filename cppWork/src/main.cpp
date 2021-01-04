using namespace std;
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include "../utils/singleStock.cpp"


//Right now this is used just for testing
int main () {
    string userInput;
    Exchanges chosenExchange = none;

    cout << "What exchange would you like to update(all, nasdaq, nyse)?: ";

    //Get the exchange from the user
    while ( chosenExchange == none) {
        //cin >> userInput;
        userInput = "nyse";

        char c;
        int i = 0;
        while (userInput[i]) {
            c = userInput[i];
            userInput[i] = (tolower(c));
            i++;
        }
        
        if (userInput.compare("all") == 0 ) {
            chosenExchange = all;
        } else if ( userInput.compare("nyse") == 0) {
            chosenExchange = nyse;
        } else if (userInput.compare("nasdaq") == 0 ) {
            chosenExchange = nasdaq;
        } else {
            cout << "Invalid, try again ";
        }
    }
    
    singleStock stonk;

    int returnVal = stonk.getStockHistory("./data/AAPL.csv");

    if (returnVal == -1) {
        cout << "Failed" << endl;
    }  else {
        cout << "File length = " << returnVal << endl;
    }

    //stonk.printVals(20, 0);

    //************* FOR A RANGE
    
    int num = 20;

    double arr[num];

    for (int i = 0; i < num; i++) {
        arr[i] = stonk.inchimokuCloudB(i);
    }

    cout << "arr = [";
    for (int i = 0; i < num; i++) {
        cout << arr[i] << ", ";
    }
    cout << "]" << endl;
    


   //************* FOR A SINGLE VALUE
   //cout << "high = " << stonk.periodHigh(returnVal, 0) << endl;

    return 0;
}