#include <iostream>

#include <string>
#include <sstream>

#include "curlpp/cURLpp.hpp"
#include "curlpp/Easy.hpp"
#include "curlpp/Options.hpp"
// #include "curlpp/Infos.hpp"
#include "curlpp/Exception.hpp"

#include "nlohmann/json.hpp"


using namespace std;

using json = nlohmann::json;


/**
 * handling requests and fetching data
 */


string fetch_data(string url, list<string> headers);


/**
 * getting a players uuid
 */


string get_player_uuid(string player_name);


/**
 * getting player data
 */


json get_player_data(string player_uuid);


/**
 * checking if players are online
 */


bool player_is_online(json player_data);
