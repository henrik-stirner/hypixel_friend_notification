#include <iostream>
#include <fstream>

#include <string>
#include <vector>

#include <chrono>
#include <thread>

#include "hypixel_api_wrapper.h"
#include "util.h"


using namespace std;

using json = nlohmann::json;


vector<string> player_names;
vector<string> player_uuids;
vector<bool> player_online_states;


/**
 * user input
 */


// prompting for player names and fetching the corresponding uuids

void get_players() {
	cout << endl
		<< "Please enter the names of the players whose online status should be checked below." << endl
		<< "Enter [Q/q] if you are done." << endl
		<< endl;

	string input;
	while (true) {
		cin >> input;

		if ((input == "Q" || input == "q") && !player_names.empty()) {
			// the user has entered all names they want to
			return;
		}

		string uuid = get_player_uuid(input);

		if (uuid == "non-existent") {
			continue;
		}

		player_names.push_back(input);
		player_uuids.push_back(uuid);
		player_online_states.push_back(false);

		cout << "| " << uuid << endl;
	}
}


// prompting for the length of the pause between requests

int get_request_interval() {
	cout << endl
		<< "Please enter the time interval to sleep between requests in SECONDS below." << endl
		<< "| The rate limit of the API are 300 queries per 5 minutes which is equivalent to 1 queries per second." << endl
		<< "| Therefore, the interval needs to be at least 1 s per player." << endl
		<< endl;

	double interval_seconds;
	double min_possible_interval_seconds = player_names.size() * 1;

	while (true) {
		cin >> interval_seconds;

		if (interval_seconds >= min_possible_interval_seconds) {
			break;
		}
		else {
			cout << "| The interval needs to be bigger than or equal to " << min_possible_interval_seconds << "." << endl;
		}
	}

	int interval_milliseconds = interval_seconds * 1000;
	return interval_milliseconds;
}


/**
 * processing player data
 */


void update_player_online_states() {
	ofstream log_file;

	auto time_01 = chrono::high_resolution_clock::now();

	int player_count = player_names.size();
	bool ratelimit_requires_small_scale_chunking = (player_count > 200);
	bool ratelimit_requires_serious_chunking = (player_count > 300);

	for (int i = 0; i < player_count; i++) {
		json player_data = get_player_data(player_uuids.at(i));
		if (player_data.is_null()) {
			continue;
		}

		// ====================================================================================================

		int player_last_login = player_data["player"]["lastLogin"].get<int>();
		int player_last_logout = player_data["player"]["lastLogout"].get<int>();
		bool player_is_online = (player_last_login > player_last_logout);

		if (player_online_states.at(i) == player_is_online) {
			// nothing changed about this players online state
			continue;
		}
		else {
			player_online_states.at(i) = player_is_online;
		}

		// inform the user about the update of the players online state
		if (player_is_online) {
			string player_most_recent_game_type = player_data["player"]["mostRecentGameType"].get<string>();
			string player_channel = player_data["player"]["channel"].get<string>();
			cout << "->- " << player_names.at(i) << " went ONLINE." << endl;
		}
		else {
			cout << "-<- " << player_names.at(i) << " went OFFLINE." << endl;
		}
		cout << "| " << current_local_time() << endl;

		// append to the log file
		log_file.open("./logs/" + player_names.at(i) + ".txt", ofstream::app);
		log_file << current_local_date_and_time() << "\t" << to_string(player_is_online) << "\n";
		log_file.close();

		// ====================================================================================================

		// the ratelimit requires us to wait for some time 
		// as we can only do a certain amount of queries in a certain amount of time
		// and the number of players exceeds this number of queries

		if (ratelimit_requires_small_scale_chunking && i % 200 == 0) {
			auto time_02 = chrono::high_resolution_clock::now();
			int time_delta = chrono::duration_cast<chrono::milliseconds>(time_02 - time_01).count();

			// we can do a maximum of 200 queries per second (-> per 1,000 milliseconds)

			if (time_delta < 1000) {
				// we have done 200 queries, so wait until a second is over before continuing
				int time_to_sleep = 1000 - time_delta;
				this_thread::sleep_for(chrono::milliseconds(time_to_sleep));
			}

			time_01 = chrono::high_resolution_clock::now();
		}

		if (ratelimit_requires_serious_chunking && i % 300 == 0) {
			auto time_02 = chrono::high_resolution_clock::now();
			int time_delta = chrono::duration_cast<chrono::milliseconds>(time_02 - time_01).count();

			// we can only do 300 queries per 5 minutes (-> per 300 seconds -> per 300,000 milliseconds)

			if (time_delta < 300000) {
				// we have done 300 queries, so wait until 5 minutes are over before continuing
				int time_to_sleep = 300000 - time_delta;
				this_thread::sleep_for(chrono::milliseconds(time_to_sleep));
			}

			time_01 = chrono::high_resolution_clock::now();
		}
	}
}


/**
 * main
 */


int main() {
	get_players();

	int request_interval_milliseconds = get_request_interval();

	cout << endl
		<< "STARTING" << endl
		<< "| Now listening for updates to the online statuses of the given players. " << endl
		<< endl;

	while (true) {
		auto time_01 = chrono::high_resolution_clock::now();

		update_player_online_states();

		auto time_02 = chrono::high_resolution_clock::now();
		int time_delta = chrono::duration_cast<chrono::milliseconds>(time_02 - time_01).count();

		if (time_delta < request_interval_milliseconds) {
			int time_to_sleep = request_interval_milliseconds - time_delta;
			this_thread::sleep_for(chrono::milliseconds(time_to_sleep));
		}
	}

	return 0;
}
