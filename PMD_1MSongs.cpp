// PMD_1MSongs.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <ctime>
#include <vector>
#include <cmath>
#include <ctime>

#include "MemoryMapped.h"
#include "sherwood_map.hpp"

#define KBYTE 1024
#define MBYTE (1024 * 1024)
#define SMALL_BLOCK_SIZE (8 * KBYTE)
#define BIG_BLOCK_SIZE (128 * KBYTE)
#define SEC_TO_YEARS 31556952 // lata przestêpne to nie jesst dok³adnie 365 dni  31536000
#define YEAR_TO_DAYS 365.2425f

using namespace std;

struct Song {
	string song_id;
	int times_played;
	string artist;
	string title;
};

struct Record {
	int song_id_int;
	int user_id_int;
	int month;
};

struct Date {
	int year;
	int month;
	int day;
};


string best_artist_song_id = "";


vector<Song> songs;
sherwood_map<int, Song> int_to_song;
sherwood_map<string, int> song_id_to_int;

vector<Record> records;
sherwood_map<string, int> user_id_to_int;
sherwood_map<int, string> int_to_user_id;

map<int, set<int>> user_unique_songs;
sherwood_map<string, int> artists_times_played;
sherwood_map<int, int> months_times_played;
sherwood_map<string, Song> queen_songs;
map<int, set<int>> users_listening_queen;

sherwood_map<int, string> song_ids_reverse;


vector<string> song_strings_to_process;
vector<string> user_strings_to_process;

string user_filepath = "f:/Downloads/triplets_sample_20p.txt";
string song_filepath = "f:/Downloads/unique_tracks.txt";

void readSequentially(const string & _filepath);
void readSequentiallyUsers(const string & _filepath);
void processSongs();
void processSongFile(const string & _data);
string truncDataToWholeLines(string & _data);
void processUsers();
void processUser(const string & _data);
void querySongRanking();
void queryUserRankingByMostUniqueSongsPlayed();
void queryArtistsByMostSongsPlayed();
void querySumOfSongsPlayedByMonth();
void queryAllUsersThatPlayedThreeMostPopularQueenSongs();

int strtoi(const string & _timestamp) {
	int sum = 0, init_max = _timestamp.length() - 1, cnt = 0;

	for (int i = init_max; i > -1; i--) {
		int num = (int)_timestamp[i] - 48;

		for (int j = 0; j < cnt; j++)
			num *= 10;

		sum += num;
		cnt++;
	}

	return sum;
}

float secs_to_years(float _secs) {
	return _secs / SEC_TO_YEARS;
}

bool isLeapYear(int _year) {
	return ((_year % 4 == 0 && _year % 100 != 0) || _year % 400 == 0);
}

int days_to_month(int _days, bool _is_leap_year) {
	if( _days <= 31 ) return 1;
	if (_days <= 59 || (_days <= 60 && _is_leap_year)) return 2;
	if (_days <= 90 || (_days <= 91 && _is_leap_year)) return 3;
	if (_days <= 120 || (_days <= 121 && _is_leap_year)) return 4;
	if (_days <= 151 || (_days <= 152 && _is_leap_year)) return 5;
	if (_days <= 181 || (_days <= 182 && _is_leap_year)) return 6;
	if (_days <= 212 || (_days <= 213 && _is_leap_year)) return 7;
	if (_days <= 243 || (_days <= 244 && _is_leap_year)) return 8;
	if (_days <= 273 || (_days <= 274 && _is_leap_year)) return 9;
	if (_days <= 304 || (_days <= 305 && _is_leap_year)) return 10;
	if (_days <= 334 || (_days <= 335 && _is_leap_year)) return 11;
	return 12;
}

int day_of_month(int _days, int _month, bool _is_leap_year) {
	if (_month == 1) return _days;
	if (_month == 2) return _days - 31;

	if (!_is_leap_year) {
		if (_month == 3) return _days - 59;
		if (_month == 4) return _days - 90;
		if (_month == 5) return _days - 120;
		if (_month == 6) return _days - 151;
		if (_month == 7) return _days - 181;
		if (_month == 8) return _days - 212;
		if (_month == 9) return _days - 243;
		if (_month == 10) return _days - 273;
		if (_month == 11) return _days - 304;
		return _days - 334;
	}

	if (_month == 3) return _days - 60;
	if (_month == 4) return _days - 91;
	if (_month == 5) return _days - 121;
	if (_month == 6) return _days - 152;
	if (_month == 7) return _days - 182;
	if (_month == 8) return _days - 213;
	if (_month == 9) return _days - 244;
	if (_month == 10) return _days - 274;
	if (_month == 11) return _days - 305;
	return _days - 335;
}

Date timestampToDate(const string & _timestamp) {
	int time = strtoi(_timestamp);
	Date d;

	float years = secs_to_years(time);

	int i_years = (int)years;
	d.year = 1970 + i_years;

	int days = ceil((years - i_years) * YEAR_TO_DAYS);
	bool is_leap_year = isLeapYear(d.year);
	d.month = days_to_month(days, is_leap_year);

	return d;
}

bool sort_func(Song & s1, Song & s2) { return s1.times_played < s2.times_played; }

void querySongRanking() {
	time_t start = clock();

	vector<Song> t_songs(songs);
	sort(t_songs.begin(), t_songs.end(), sort_func);

	int rank = 1;

	for (int i = t_songs.size() - 1; i > t_songs.size() - 11; i--) {
		cout << "Rank " << rank++ << ": " << t_songs[i].artist << " - " << t_songs[i].title << " = " << t_songs[i].times_played << endl;
	}

	cout << "Query 1 time = " << (float)(clock() - start) / CLOCKS_PER_SEC * 1000 << " ms" << endl;
}

bool sort_func2(pair<int, set<int>> const & s1, pair<int, set<int>> const & s2) {
	return s1.second.size() > s2.second.size();
}

void queryUserRankingByMostUniqueSongsPlayed() {
	time_t start = clock();

	vector<pair<int, set<int>>> pairs(user_unique_songs.size());
	int i = 0;

	for (auto it = user_unique_songs.begin(); it != user_unique_songs.end(); it++)
		pairs[i++] = *it;

	sort(pairs.begin(), pairs.end(), sort_func2);

	for (i = 0; i < 10; i++) {
		cout << "Rank " << i << ": (" << int_to_user_id[pairs[i].first] << ", " << pairs[i].second.size() << ")" << endl;
	}

	cout << "Query 2 time = " << (float)(clock() - start) / CLOCKS_PER_SEC * 1000 << " ms" << endl;
}


bool sort_func3(pair<string, int> const & s1, pair<string, int> const & s2) {
	return s1.second > s2.second;
}

void queryArtistsByMostSongsPlayed() {
	time_t start = clock();

	vector<pair<string, int>> pairs(artists_times_played.size());
	int i = 0;

	for (auto it = artists_times_played.begin(); it != artists_times_played.end(); it++)
		pairs[i++] = *it;

	sort(pairs.begin(), pairs.end(), sort_func3);

	cout << "Most played artist is: " << pairs[0].first << " - " << pairs[0].second << " plays" << endl;

	cout << "Query 3 time = " << (float)(clock() - start) / CLOCKS_PER_SEC * 1000 << " ms" << endl;
}

void querySumOfSongsPlayedByMonth() {
	time_t start = clock();

	for (int i = 1; i <= 12; i++) {
		cout << "At month " << i << " songs were played " << months_times_played[i] << " times " << endl;
	}

	cout << "Query 4 time = " << (float)(clock() - start) / CLOCKS_PER_SEC * 1000 << " ms" << endl;
}

bool sort_func4(pair<string, Song> const & s1, pair<string, Song> const & s2) {
	return s1.second.times_played > s2.second.times_played;
}

void queryAllUsersThatPlayedThreeMostPopularQueenSongs() {
	time_t start = clock();

	// wybieramy 3 najlepsze piosenki queen

	vector<pair<string, Song>> pairs(queen_songs.size());
	int i = 0;

	for (auto it = queen_songs.begin(); it != queen_songs.end(); it++)
		pairs[i++] = *it;

	sort(pairs.begin(), pairs.end(), sort_func4);
	unordered_set<int> best_3;
	cout << "Best 3 Queen songs are: " << endl;
	for (int i = 0; i < 3; i++) {
		cout << pairs[i].second.artist << " - " << pairs[i].second.title << ": " << pairs[i].second.times_played << endl;
		best_3.insert(song_id_to_int[pairs[i].first]);
	}

	vector<int> users_queen;
	for (auto it = users_listening_queen.begin(); it != users_listening_queen.end(); it++) {
		int cnt = 0;

		for (auto it2 = best_3.begin(); it2 != best_3.end(); it2++) {
			for (auto it3 = it->second.begin(); it3 != it->second.end(); it3++) {
				if (*it2 == *it3) {
					cnt++;
					break;
				}
			}
		}

		if (cnt == 3)
			users_queen.push_back(it->first);
	}

	cout << "Users which listen 3 most popular songs of Queen (" << users_queen.size() << " users): " << endl;
	for (int i = 0; i < users_queen.size(); i++)
		cout << int_to_user_id[users_queen[i]] << endl;

	cout << "Query 5 time = " << (float)(clock() - start) / CLOCKS_PER_SEC * 1000 << " ms" << endl;
}

int main(int argc, char *argv[]) {
	double time = clock();

	song_id_to_int.reserve(1100000);
	user_id_to_int.reserve(1100000);
	int_to_user_id.reserve(1100000);
	songs.resize(1000000);
	//records.resize(30000000);

	readSequentially(song_filepath);
	processSongs(); // hotspot

	readSequentiallyUsers(user_filepath);
	processUsers();

	user_strings_to_process.clear();

	querySongRanking();
	queryUserRankingByMostUniqueSongsPlayed();
	queryArtistsByMostSongsPlayed();
	querySumOfSongsPlayedByMonth();
	queryAllUsersThatPlayedThreeMostPopularQueenSongs();

	cout << (clock() - time) / CLOCKS_PER_SEC * 1000 << " ms" << endl;
}

void readSequentiallyUsers(const string &_filepath) {
	MemoryMapped file_mmf(_filepath, MemoryMapped::MapRange::WholeFile, MemoryMapped::CacheHint::SequentialScan);

	if (!file_mmf.isValid()) {
		cout << "Nie wczytano pliku " << _filepath << endl;
		return;
	}

	const unsigned char * raw_data = file_mmf.getData();
	unsigned int pos = 0, cnt = 0, x = 0;
	string remainder = "";
	int texts_number = floor((double)file_mmf.size()) / BIG_BLOCK_SIZE + 1;

	cout << "tnum = " << texts_number << endl;

	user_strings_to_process.resize(texts_number);

	do {
		char buffer[BIG_BLOCK_SIZE];
		unsigned int bytes = BIG_BLOCK_SIZE;

		if (pos == texts_number - 1) {
			unsigned int diff = (texts_number - 1) * BIG_BLOCK_SIZE;
			bytes = file_mmf.size() - diff + 1;
		}

		memcpy(buffer, raw_data + x, bytes);
		x += bytes;

		string file_data(buffer, bytes);

		file_data = remainder + file_data;

		remainder = truncDataToWholeLines(file_data);


		// hotspot - 20 ms
		user_strings_to_process[cnt++] = file_data;
	} while (pos++ < texts_number - 1);

	//cout << user_strings_to_process[texts_number - 1] << endl;
}

void processUser(const string & _data) {
	int l = _data.length();

	if (l == 0)
		return;

	const int USER_ID_LEN = 40, SONG_ID_LEN = 18, SEP_GAP = 5, MIN_TIMESTAMP_LEN = 9;
	int start_pos = 0, end_pos = 0, cnt = 0;

	do {
		end_pos += USER_ID_LEN;
		string user_id = _data.substr(start_pos, end_pos - start_pos);
		start_pos = end_pos + SEP_GAP;

		end_pos = start_pos + SONG_ID_LEN;
		string song_id = _data.substr(start_pos, end_pos - start_pos);
		start_pos = end_pos + SEP_GAP;

		end_pos = _data.find('\n', start_pos + MIN_TIMESTAMP_LEN);	
		string timestamp = _data.substr(start_pos, end_pos - start_pos);
		Date date = timestampToDate(timestamp);

		//cout << date.day << "/" << date.month << "/" << date.year << endl;
		start_pos = end_pos = end_pos + 1;

		//cout << "song_id = " << song_id << endl << "artist = " << artist << endl << "song = " << song << endl << endl;

		int user_id_int = user_id_to_int.size();
		if (!user_id_to_int[user_id]) {
			user_id_to_int[user_id] = user_id_int;
			int_to_user_id[user_id_int] = user_id;
		}
		else {
			user_id_int = user_id_to_int[user_id];
		}

		Record r;
		r.month = date.month;
		r.song_id_int = song_id_to_int[song_id];
		r.user_id_int = user_id_int;

		// 1. nalicz odtworzon¹ piosenkê
		songs[r.song_id_int].times_played++;

		// 2. dodaj u¿ytkownikowi odtworzon¹ piosenkê
		user_unique_songs[user_id_int].insert(r.song_id_int);

		// 3. najczêœciej grany artysta
		if (!artists_times_played[songs[r.song_id_int].artist])
			artists_times_played[songs[r.song_id_int].artist] = 1;
		else
			artists_times_played[songs[r.song_id_int].artist]++;

		// 4. dodajemy w którym miesi¹cu grali
		if (!months_times_played[r.month])
			months_times_played[r.month] = 1;
		else
			months_times_played[r.month]++;

		// 5. userzy którz s³uchali 3 najpopularniejszych utworów Queen
		if (songs[r.song_id_int].artist == "Queen") {
			queen_songs[song_id] = songs[r.song_id_int];
			users_listening_queen[r.user_id_int].insert(r.song_id_int);
		}

		//records[cnt++] = r;
	} while (end_pos < l - 2);
}

void processUsers() {
	for (int i = 0; i < user_strings_to_process.size(); i++)
		processUser(user_strings_to_process[i]);

}

void readSequentially(const string & _filepath) {
	// open file
	MemoryMapped file_mmf(_filepath, MemoryMapped::MapRange::WholeFile, MemoryMapped::CacheHint::SequentialScan);

	if (!file_mmf.isValid()) {
		cout << "Nie wczytano pliku " << _filepath << endl;
		return;
	}

	const unsigned char * raw_data = file_mmf.getData();
	int pos = 0, cnt = 0, x = 0;

	string remainder = "";
	int texts_number = floor((double)file_mmf.size() / SMALL_BLOCK_SIZE) + 1;

	song_strings_to_process.resize(texts_number);

	do {
		char buffer[SMALL_BLOCK_SIZE];
		int bytes = SMALL_BLOCK_SIZE;

		if (pos == texts_number - 1) {
			bytes = file_mmf.size() - (texts_number - 1) * SMALL_BLOCK_SIZE + 1;
		}

		memcpy(buffer, raw_data + x, bytes);
		x += bytes;

		string file_data(buffer, bytes);

		file_data = remainder + file_data;

		remainder = truncDataToWholeLines(file_data);

		song_strings_to_process[cnt++] = file_data;
	} while (pos++ < texts_number - 1);

}

void processSongs() {
	for (int i = 0; i < song_strings_to_process.size() - 1; i++) {
		processSongFile(song_strings_to_process[i]);
	}

	cout << "songs = " << song_id_to_int.size() << endl;
}

void processSongFile(const string & _data) {
	int l = _data.length();

	if (l == 0)
		return;

	const int SONG_ID_GAP = 23, SONG_ID_LEN = 18, SONG_ID_ARTIST_GAP = 28, SEP_GAP = 5;
	int start_pos = SONG_ID_GAP, end_pos = SONG_ID_GAP;

	do {
		end_pos += SONG_ID_LEN;
		string song_id = _data.substr(start_pos, end_pos - start_pos);
		start_pos += SONG_ID_LEN + SEP_GAP;

		end_pos = _data.find('<', start_pos);
		string artist = _data.substr(start_pos, end_pos - start_pos);
		start_pos = end_pos + SEP_GAP;

		end_pos = _data.find('\n', start_pos);
		string title = _data.substr(start_pos, end_pos - start_pos);
		start_pos = end_pos = end_pos + 1 + SONG_ID_GAP;

		//cout << "song_id = " << song_id << endl << "artist = " << artist << endl << "song = " << song << endl << endl;
		
		Song s;
		s.artist = artist;
		s.title = title;
		s.times_played = 0;
		
		int song_id_int = song_id_to_int.size();
		if (!song_id_to_int[song_id]) {
			song_id_to_int[song_id] = song_id_int;
			songs[song_id_int] = s;
		}



	} while (end_pos < l - 2);
}


string truncDataToWholeLines(string & _data) {
	int cnt = 0, l = _data.length() - 1;

	while (_data[l--] != '\n')
		cnt++;

	if (cnt == 0)
		return "";

	int pos = l + 2;
	string rem = _data.substr(pos);
	_data = _data.substr(0, pos);

	//cout << "data = " << _data << endl << "rem = " << rem << endl << endl;

	return rem;
}

