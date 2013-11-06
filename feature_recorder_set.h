/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#ifndef FEATURE_RECORDER_SET_H
#define FEATURE_RECORDER_SET_H

#include "feature_recorder.h"
#include "cppmutex.h"
#include "dfxml/src/hash_t.h"

/** \addtogroup internal_interfaces
 * @{
 */
/** \file */

/**
 * \class feature_recorder_set
 * A singleton class that holds a set of recorders.
 * This used to be done with a set, but now it's done with a map.
 * 
 */
#include <map>
#include <set>

typedef std::map<string,class feature_recorder *> feature_recorder_map;
typedef std::set<string>feature_file_names_t;
class feature_recorder_set {
    // neither copying nor assignment is implemented 
    feature_recorder_set(const feature_recorder_set &fs);
    feature_recorder_set &operator=(const feature_recorder_set &fs);
    uint32_t flags;
    atomic_set<std::string> seen_set;   // hex hash values of pages that have been seen
public:
    struct pstats {
        double seconds;
        uint64_t calls;
    };
    typedef map<std::string,struct pstats> scanner_stats_map;

private:
    // instance data //
    std::string           input_fname;      // input file
    std::string           outdir;           // where output goes
public:
    feature_recorder_map  frm;              // map of feature recorders, by name
    cppmutex              map_lock;               // locks frm and scanner_stats_map
    scanner_stats_map     scanner_stats;

public:
    static const string   ALERT_RECORDER_NAME;  // the name of the alert recorder
    static const string   DISABLED_RECORDER_NAME; // the fake disabled feature recorder
    /* flags */
    static const uint32_t ONLY_ALERT=0x01;      // always return the alert recorder
    static const uint32_t SET_DISABLED=0x02;    // the set is effectively disabled; for path-printer
    static const uint32_t CREATE_STOP_LIST_RECORDERS=0x04; // 

    virtual ~feature_recorder_set() {
        for(feature_recorder_map::iterator i = frm.begin();i!=frm.end();i++){
            delete i->second;
        }
    }

    /** create an emptry feature recorder set. If disabled, create a disabled recorder. */
    feature_recorder_set(uint32_t flags_);

    /** Initialize a feature_recorder_set. Previously this was a constructor, but it turns out that
     * virtual functions for the create_name_factory aren't honored in constructors.
     */
    void init(const feature_file_names_t &feature_files,
              const std::string &input_fname,const std::string &outdir);

    void flush_all();
    void close_all();
    bool has_name(string name) const;           /* does the named feature exist? */
    void set_flag(uint32_t f){flags|=f;}         
    void clear_flag(uint32_t f){flags|=f;}

    virtual feature_recorder *create_name_factory(const std::string &outdir_,const std::string &input_fname_,const std::string &name_);
    virtual void create_name(const std::string &name,bool create_stop_also);
    virtual const std::string &get_outdir(){ return outdir;}

    void add_stats(string bucket,double seconds);
    typedef void (*stat_callback_t)(void *user,const std::string &name,uint64_t calls,double seconds);
    void get_stats(void *user,stat_callback_t stat_callback);

    // Management of previously seen data
public:
    virtual bool check_previously_processed(const uint8_t *buf,size_t bufsize);

    // NOTE:
    // only virtual functions may be called by plugins!
    virtual feature_recorder *get_name(const std::string &name);
    virtual feature_recorder *get_alert_recorder();
};


#endif
