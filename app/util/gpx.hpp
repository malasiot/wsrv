#pragma once

#include <string>
#include <optional>
#include <vector>
#include <iomanip>
#include "xml_pull_parser.hpp"

struct TrackPoint {
    double lat_ = 0.0;
    double lon_ = 0.0;
    std::optional<int64_t> time_ ;
    std::optional<double> ele_;
};

struct WayPoint {
    double lat_ = 0.0;
    double lon_ = 0.0;
    std::optional<int64_t> time_ ;
    std::optional<double> ele_;
    std::string name_ ;
    std::string desc_ ;
};

struct TrackSegment {
    std::vector<TrackPoint> points_ ;
};

struct Track {
    std::string name_ = "";
    std::vector<TrackSegment> segments_;
};

struct GPX {
    
    std::string creator_ = "";
    std::vector<WayPoint> waypoints_;
    std::vector<Track> tracks_;
};


class GPXParser {
public:
    bool parse(std::istream &strm, GPX &gpx) {
        Track current_track;
        TrackPoint current_pt;
        WayPoint current_wpt ;
        TrackSegment current_seg ;
        std::string current_element = "";
        
        bool in_track = false;
        bool in_waypoint = false; // Shared for <wpt> and <trkpt>
        bool in_trkpoint = false;
        bool in_trkseg = false ;

        try {
            XmlPullParser reader(strm) ;

            int eventType = reader.getEventType();
            while ( eventType !=  XmlPullParser::END_DOCUMENT ) {
                
                switch (eventType) {
                    case XmlPullParser::START_TAG: {
                        current_element = reader.getName();

                        if (current_element == "gpx") {
                            gpx.creator_ = reader.getAttribute("creator");
                        } else if (current_element == "trk") {
                            in_track = true;
                            current_track = Track(); // Reset container
                        } else if (current_element == "trkseg") {
                            in_trkseg = true;
                            current_seg = TrackSegment(); // Reset container
                        } else if (current_element == "wpt") {
                            in_waypoint = true;
                            current_wpt = WayPoint(); // Reset container
                            current_wpt.lat_ = std::stod(reader.getAttribute("lat"));
                            current_wpt.lon_ = std::stod(reader.getAttribute("lon"));
                        } else if (current_element == "trkpt") {
                            in_trkpoint = true;
                            current_pt = TrackPoint(); // Reset container
                            current_pt.lat_ = std::stod(reader.getAttribute("lat"));
                            current_pt.lon_ = std::stod(reader.getAttribute("lon"));
                        }
                        break;
                    }

                    case XmlPullParser::TEXT: {
                      
                        std::string text = reader.getText();
                        if (current_element == "ele") {
                            if ( in_waypoint )
                                current_wpt.ele_ = std::stod(text);
                            else if ( in_trkpoint )
                                current_pt.ele_ = std::stod(text);
                        } 
                        else if ( current_element == "time" ) {
                            if ( in_waypoint )
                                current_wpt.time_ = parse_date_time(text);
                            else if ( in_trkpoint )
                                current_pt.time_ = parse_date_time(text) ;
                        }
                        else if ( current_element == "name" ) {
                            if ( in_waypoint )
                                current_wpt.name_ = text ;
                            else if ( in_track )
                                current_track.name_ = text ;
                        }
                        break;
                    }

                    case XmlPullParser::END_TAG: {
                        std::string end_tag = reader.getName();

                        if ( end_tag == "trkpt" ) {
                            current_seg.points_.push_back(current_pt);
                            in_trkpoint = false;
                        } else if ( end_tag == "wpt" ) {
                            gpx.waypoints_.push_back(current_wpt);
                            in_waypoint = false;
                        } else if (  end_tag == "trkseg" ) {
                            current_track.segments_.push_back(current_seg);
                            in_track = false;
                        } else if (  end_tag == "trk" ) {
                            gpx.tracks_.push_back(current_track);
                            in_track = false;
                        }
                        
                        // Reset current element context to prevent characters event misfires
                        current_element = ""; 
                        break;
                    }
                    
                    default:
                        break;
                }
                eventType = reader.next();
            }
            return true;
        } catch ( XmlPullParserException &e ) {
            return false ;
        }
    }

    static int64_t parse_date_time(const std::string &src) {
        std::istringstream ss(src);
        std::tm tm = {};
        
        // 1. Parse the main date and time components up to seconds
        // Handles standard format: "2026-06-09T16:08:00"
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if (ss.fail()) {
            return 0;
        }

        // 2. Handle sub-seconds if present (e.g., ".123Z" or ".456789Z")
        char nextChar = ss.peek();
        if (nextChar == '.') {
            ss.get(); // Consume the '.'
            double fractionalSeconds;
            // Read the trailing float (this moves the stream position past the fraction)
            if (!(ss >> fractionalSeconds)) {
                // If reading fraction fails, keep going with whole seconds
                ss.clear();
            }
        }

        // 3. Convert std::tm (interpreted strictly as UTC) to Epoch seconds.
        // std::mktime assumes the local system timezone, so we must compensate.
        
        // Query local timezone offset relative to UTC for this specific historical date
        std::tm localTm = tm;
        std::time_t localTime = std::mktime(&localTm);
        if (localTime == -1) return 0;

#if defined(_WIN32)
        std::tm gmTm;
        if (gmtime_s(&gmTm, &localTime) != 0) return 0;
#else
        std::tm gmTm;
        if (gmtime_r(&localTime, &gmTm) == nullptr) return 0;
#endif

        // Calculate the difference between local system interpretation and true GMT
        std::time_t offset = localTime - std::mktime(&gmTm);
        
        // Absolute absolute epoch time in seconds
        return static_cast<int64_t>(localTime + offset);
    }
};
