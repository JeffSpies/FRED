/*
  This file is part of the FRED system.

  Copyright (c) 2013-2016, University of Pittsburgh, John Grefenstette,
  David Galloway, Mary Krauland, Michael Lann, and Donald Burke.

  Based in part on FRED Version 2.9, created in 2010-2013 by
  John Grefenstette, Shawn Brown, Roni Rosenfield, Alona Fyshe, David
  Galloway, Nathan Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Workplace.h
//

#ifndef _FRED_WORKPLACE_H
#define _FRED_WORKPLACE_H

#include <climits>
#include <vector>

#include "Place.h"

class Office;

class Workplace : public Place {
public: 

  /**
   * Default constructor
   */
  Workplace();

  /**
   * Constructor with necessary parameters
   */
  Workplace(const char* lab, char _subtype, fred::geo lon, fred::geo lat);

  ~Workplace() {}

  static void get_parameters();

  /**
   * Initialize the workplace and its offices
   */
  void prepare();

  /**
   * @see Place::get_group(int condition, Person* per)
   */
  int get_group(int condition, Person* per) {
    return 0;
  }

  /**
   * @see Place::get_transmission_prob(int condition, Person* i, Person* s)
   *
   * This method returns the value from the static array <code>Workplace::Workplace_contact_prob</code> that
   * corresponds to a particular age-related value for each person.<br />
   * The static array <code>Workplace_contact_prob</code> will be filled with values from the parameter
   * file for the key <code>workplace_prob[]</code>.
   */
  double get_transmission_prob(int condition, Person* i, Person* s);

  /**
   * @see Place::get_contacts_per_day(int condition)
   *
   * This method returns the value from the static array <code>Workplace::Workplace_contacts_per_day</code>
   * that corresponds to a particular condition.<br />
   * The static array <code>Workplace_contacts_per_day</code> will be filled with values from the parameter
   * file for the key <code>workplace_contacts[]</code>.
   */
  double get_contacts_per_day(int condition);

  int get_number_of_rooms();
    
  /**
   * Setup the offices within this Workplace
   */
  void setup_offices();

  /**
   * Assign a person to a particular Office
   * @param per the Person to assign
   * @return the Office to which the Person was assigned
   */
  Place* assign_office(Person* per);

  int get_workplace_size_group_id() {
    for(int i = 0; i < Workplace::get_workplace_size_group_count(); ++i) {
      if(i <= Workplace::get_workplace_size_max_by_group_id(i)) {
        return i;
      }
    }
    return -1;
  }

  /**
   * Determine if the Workplace should be open. It is dependent on the condition and simulation day.
   *
   * @param day the simulation day
   * @param condition an integer representation of the condition
   * @return whether or not the workplace is open on the given day for the given condition
   */
  bool should_be_open(int day, int condition) {
    return true;
  }

  static int get_max_office_size() {
    return Workplace::Office_size;
  }

  static int get_total_workers() {
    return Workplace::total_workers;
  }
    
  // for access from Office
  static double get_workplace_contacts_per_day(int condition_id) {
    return Workplace::contacts_per_day;
  }

  static int get_workplace_size_group_count() {
    return Workplace::workplace_size_group_count;
  }

  static int get_workplace_size_max_by_group_id(int group_id) {
    if(group_id < 0 || group_id > Workplace::get_workplace_size_group_count()) {
      return -1;
    } else {
      return Workplace::workplace_size_max[group_id];
    }
  }

  static int get_count_workers_by_workplace_size(int group_id) {
    if(group_id < 0 || group_id > Workplace::workplace_size_group_count) {
      return -1;
    } else {
      return Workplace::workers_by_workplace_size[group_id];
    }
  }

  int get_number_of_offices() {
    return this->offices.size();
  }

  Office* get_office(int i) {
    if (0 <= i && i < get_number_of_offices()) {
      return this->offices[i];
    } else {
      return NULL;
    }
  }
private:
  static double contacts_per_day;
  static double** prob_transmission_per_contact;
  static int Office_size;
  static int total_workers;
  static std::vector<int> workplace_size_max; // vector to hold the upper limit for each workplace size group
  static std::vector<int> workers_by_workplace_size; // vector to hold the counts of workers in each group (plus, the "greater than" group)
  static int workplace_size_group_count;
  vector<Office*> offices;
  int next_office;
};

#endif // _FRED_WORKPLACE_H

