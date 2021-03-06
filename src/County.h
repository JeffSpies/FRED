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
// File: County.h
//

#ifndef _FRED_COUNTY_H
#define _FRED_COUNTY_H

#include <vector>
using namespace std;

#include "Demographics.h"
class Household;
class Person;
class Place;
class School;
class Workplace;


// 2-d array of lists
typedef std::vector<int> HouselistT;

// number of age groups: 0-4, 5-9, ... 85+
#define AGE_GROUPS 18

// number of target years: 2010, 2015, ... 2040
#define TARGET_YEARS 7

class County {
public:

  County(int _fips);
  ~County();

  void setup();

  int get_fips() {
    return this->fips;
  }

  int get_number_of_households() {
    return this->number_of_households;
  }

  int get_current_popsize() {
    return this->tot_current_popsize;
  }

  int get_tot_female_popsize() {
    return this->tot_female_popsize;
  }

  int get_tot_male_popsize() {
    return this->tot_male_popsize;
  }

  int get_current_popsize(int age) {
    if(age > Demographics::MAX_AGE) {
      age = Demographics::MAX_AGE;
    }
    if(age >= 0) {
      return this->female_popsize[age] + this->male_popsize[age];
    }
    return -1;
  }

  int get_current_popsize(int age, char sex) {
    if(age > Demographics::MAX_AGE) {
      age = Demographics::MAX_AGE;
    }
    if(age >= 0) {
      if(sex == 'F') {
        return this->female_popsize[age];
      } else if(sex == 'M') {
        return this->male_popsize[age];
      }
    }
    return -1;
  }

  int get_current_popsize(int age_min, int age_max, char sex);
  bool increment_popsize(Person* person);
  bool decrement_popsize(Person* person);
  void recompute_county_popsize();

  void add_household(Household* h) {
    this->households.push_back(h);
  }
  
  void update(int day);
  void get_housing_imbalance(int day);
  int fill_vacancies(int day);
  void update_housing(int day);
  void move_college_students_out_of_dorms(int day);
  void move_college_students_into_dorms(int day);
  void move_military_personnel_out_of_barracks(int day);
  void move_military_personnel_into_barracks(int day);
  void move_inmates_out_of_prisons(int day);
  void move_inmates_into_prisons(int day);
  void move_patients_into_nursing_homes(int day);
  void move_young_adults(int day);
  void move_older_adults(int day);
  void report_ages(int day, int house_id);
  void swap_houses(int day);
  int find_fips_code(int n);
  void get_housing_data();
  void report_household_distributions();
  void report_county_population();

  double get_pregnancy_rate(int age) {
    return this->pregnancy_rate[age];
  }

  double get_mortality_rate(int age, char sex) { 
    if(sex == 'F') {
      if(age > Demographics::MAX_AGE) {
        return this->adjusted_female_mortality_rate[Demographics::MAX_AGE];
      } else {
        return this->adjusted_female_mortality_rate[age];
      }
    } else {
      if(age > Demographics::MAX_AGE) {
        return this->adjusted_male_mortality_rate[Demographics::MAX_AGE];
      } else {
        return this->adjusted_male_mortality_rate[age];
      }
    }
  } 

  // for selecting new workplaces
  void set_workplace_probabilities();
  Workplace* select_new_workplace();
  void report_workplace_sizes();

  // for selecting new schools
  void set_school_probabilities();
  School* select_new_school(int grade);
  void report_school_sizes();

  // county migration
  void read_population_target_parameters();
  void county_to_county_migration();
  Household* get_household(int i) {
    assert(0 <= i && i < this->households.size());
    return this->households[i];
  }
  void migrate_household_to_county(Place* house, int dest);
  Place* select_new_house_for_immigrants(int hszie);
  void select_migrants(int day, int migrants, int lower_age, int upper_age, char sex, int dest_fips);
  void add_immigrant(Person* person);
  void add_immigrant(int age, char sex);
  void migration_swap_houses();
  void migrate_to_target_popsize();
  void read_migration_parameters();
  double get_migration_rate(int sex, int age, int src, int dst);
  void group_population_by_sex_and_age(int reset);
  void report();

private:
  int fips;
  int tot_current_popsize;
  int male_popsize[Demographics::MAX_AGE + 1];
  int tot_male_popsize;
  int female_popsize[Demographics::MAX_AGE + 1];
  int tot_female_popsize;

  double male_mortality_rate[Demographics::MAX_AGE + 1];
  double female_mortality_rate[Demographics::MAX_AGE + 1];
  double mortality_rate_adjustment_weight;
  double adjusted_male_mortality_rate[Demographics::MAX_AGE + 1];
  double adjusted_female_mortality_rate[Demographics::MAX_AGE + 1];
  double birth_rate[Demographics::MAX_AGE + 1];
  double adjusted_birth_rate[Demographics::MAX_AGE + 1];
  double pregnancy_rate[Demographics::MAX_AGE + 1];
  bool is_initialized;
  double population_growth_rate;
  double college_departure_rate;
  double military_departure_rate;
  double prison_departure_rate;
  double youth_home_departure_rate;
  double adult_home_departure_rate;
  int* beds;
  int* occupants;
  int max_beds;
  std::vector< pair<Person*, int> > ready_to_move;
  int target_males[AGE_GROUPS][TARGET_YEARS];
  int target_females[AGE_GROUPS][TARGET_YEARS];
  std::vector<Person*> males_of_age[Demographics::MAX_AGE+1];
  std::vector<Person*> females_of_age[Demographics::MAX_AGE+1];

  // pointers to households
  std::vector<Household*> households;
  int number_of_households;

  // pointers to nursing homes
  std::vector<Household*> nursing_homes;
  int number_of_nursing_homes;

  // schools attended by people in this county, with probabilities
  std::vector<School*> schools_attended[Global::GRADES];
  std::vector<double> school_probabilities[Global::GRADES];

  // workplaces attended by people in this county, with probabilities
  std::vector<Workplace*> workplaces_attended;
  std::vector<double> workplace_probabilities;

  std::vector<int> migration_households;  //vector of household IDs for migration
 
  // static vars
  static int enable_migration_to_target_popsize;
  static int enable_county_to_county_migration;
  static int enable_within_state_school_assignment;
  static std::vector<int> migration_fips;
  static double**** migration_rate;
  static int migration_parameters_read;
  static int population_target_parameters_read;
  static int*** male_migrants;
  static int*** female_migrants;

};

#endif // _FRED_COUNTY_H
