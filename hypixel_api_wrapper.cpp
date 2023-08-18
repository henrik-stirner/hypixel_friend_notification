/**
 *    Copyright (c) <2002-2005> <Jean-Philippe Barrette-LaPierre>
 *
 *    Permission is hereby granted, free of charge, to any person obtaining
 *    a copy of this software and associated documentation files
 *    (curlpp), to deal in the Software without restriction,
 *    including without limitation the rights to use, copy, modify, merge,
 *    publish, distribute, sublicense, and/or sell copies of the Software,
 *    and to permit persons to whom the Software is furnished to do so,
 *    subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included
 *    in all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 *	  REFERENCE: 
 *	  https://github.com/datacratic/curlpp/blob/master/examples/example20.cpp
 */


#include "hypixel_api_wrapper.h"

#include "api_key.h"


/**
 * hypixel-specific
 */

const string PLAYER_UUID_URL = "https://api.mojang.com/users/profiles/minecraft/";  // -> append player name
const string PLAYER_DATA_URL = "https://api.hypixel.net/player?uuid=";  // -> append player uuid; put API-Key in header


/**
 * handling requests and fetching data
 */

string fetch_data(string url, list<string> headers) {
	std::ostringstream data;

	try {
		curlpp::Cleanup cleaner;
		curlpp::Easy request;

		request.setOpt(new curlpp::options::WriteStream(&data));

		request.setOpt(new curlpp::options::Url(url));
		request.setOpt(new curlpp::options::Verbose(false));
		if (headers.size() > 0) {
			request.setOpt(new curlpp::options::HttpHeader(headers));
		}

		request.perform();
	}
	catch (curlpp::RuntimeError& e) {
		cout << e.what() << endl;
		return "";
	}
	catch (curlpp::LogicError& e) {
		cout << e.what() << endl;
		return "";
	}

	cout << data.str() << endl;
	return data.str();
}


/**
 * getting a players uuid
 */


string get_player_uuid(string player_name) {
	string url = PLAYER_UUID_URL + player_name;
	
	list<string> headers;

	string raw_data = fetch_data(url, headers);

	if (raw_data == "") {
		cout << "| An error occured while trying to fetch this players uuid. Please try again." << endl;
		return "non-existent";
	}

	json data = json::parse(raw_data);

	if (data.contains("errorMessage")) {
		// the server is telling us that something went wrong
		cout << "| This player does not exist." << endl
			<< "| Error message: " << data.at("errorMessage") << endl;
		return "non-existent";
	}

	// we got the uuid
	return data.at("id");
}


/**
 * getting player data
 */


json get_player_data(string player_uuid) {
	string url = PLAYER_DATA_URL + player_uuid;

	list<string> headers;
	headers.push_back("API-Key: " + HYPIXEL_API_KEY);

	string raw_data = fetch_data(url, headers);

	if (raw_data == "") {
		cout << "| An error occured while trying to fetch data from " + url << endl;
		
		json null_json;
		return null_json;
	}

	json data = json::parse(raw_data);

	if (!data.at("success")) {
		cout << "| An error occured while trying to fetch data from " << url << endl
			<< "| Cause: " << data.at("cause") << endl;
		
		json null_json;
		return null_json;
	}

	return data;
}


/**
 * checking if a player is online
 */


bool player_is_online(json player_data) {
	int player_last_login = player_data["player"]["lastLogin"].get<int>();
	int player_last_logout = player_data["player"]["lastLogout"].get<int>();

	return (player_last_login > player_last_logout);
}


// string player_most_recent_game_type = player_data["player"]["mostRecentGameType"].get<string>();
// string player_channel = player_data["player"]["channel"].get<string>();
