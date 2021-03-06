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
// File: Fred.cc
//
#include "Activities.h"
#include "Date.h"
#include "Demographics.h"
#include "Condition.h"
#include "Condition_List.h"
#include "Epidemic.h"
#include "Fred.h"
#include "Global.h"
#include "Health.h"
#include "Logit.h"
#include "Neighborhood_Layer.h"
#include "Network.h"
#include "Params.h"
#include "Place_List.h"
#include "Population.h"
#include "Random.h"
#include "Regional_Layer.h"
#include "Seasonality.h"
#include "Sexual_Transmission_Network.h"
#include "Tracker.h"
#include "Transmission.h"
#include "Travel.h"
#include "Utils.h"
#include "Vector_Layer.h"
#include "Visualization_Layer.h"
class Place;

#ifndef __CYGWIN__
#include "execinfo.h"
#endif /* __CYGWIN__ */

#include <csignal>
#include <cstdlib>
#include <cxxabi.h>


//FRED main program

int main(int argc, char* argv[]) {
  fred_setup(argc, argv);
  for(Global::Simulation_Day = 0; Global::Simulation_Day < Global::Days; Global::Simulation_Day++) {
    fred_step(Global::Simulation_Day);
  }
  fred_finish();
  return 0;
}


void fred_setup(int argc, char* argv[]) {
  char paramfile[FRED_STRING_SIZE];

  Global::Simulation_Day = 0;
  Global::Statusfp = stdout;
  Utils::fred_print_wall_time("FRED started");
  Utils::fred_start_initialization_timer();
  Utils::fred_start_timer();

  // read optional param file name from command line
  if(argc > 1) {
    strcpy(paramfile, argv[1]);
  } else {
    strcpy(paramfile, "params");
  }
  fprintf(Global::Statusfp, "param file = %s\n", paramfile);
  fflush(Global::Statusfp);

  // read optional run number from command line (must be 2nd arg)
  if(argc > 2) {
    sscanf(argv[2], "%d", &Global::Simulation_run_number);
  } else {
    Global::Simulation_run_number = 1;
  }

  // read optional working directory from command line (must be 3rd arg)
  if(argc > 3) {
    strcpy(Global::Simulation_directory, argv[3]);
  } else {
    strcpy(Global::Simulation_directory, "");
  }

  // get runtime parameters
  Params::read_parameters(paramfile);
  Global::get_global_parameters();
  Date::setup_dates(Global::Start_date);

  // create conditions and read parameters
  Global::Conditions.get_parameters();
  Transmission::get_parameters();

  Global::Pop.get_parameters();
  Utils::fred_print_lap_time("get_parameters");

  if(strcmp(Global::Simulation_directory, "") == 0) {
    // use the directory in the params file
    strcpy(Global::Simulation_directory, Global::Output_directory);
    //printf("Global::Simulation_directory = %s\n",Global::Simulation_directory);
  } else {
    // change the Output_directory
    strcpy(Global::Output_directory, Global::Simulation_directory);
    FRED_STATUS(0, "Overridden from command line: Output_directory = %s\n",
		Global::Output_directory);
  }

  // create the output directory, if necessary
  Utils::fred_make_directory(Global::Simulation_directory);

  // open output files with global file pointers
  Utils::fred_open_output_files();

  // set random number seed based on run number
  if(Global::Simulation_run_number > 1 && Global::Reseed_day == -1) {
    Global::Simulation_seed = Global::Seed * 100 + (Global::Simulation_run_number - 1);
  } else {
    Global::Simulation_seed = Global::Seed;
  }
  fprintf(Global::Statusfp, "seed = %lu\n", Global::Simulation_seed);
  Random::set_seed(Global::Simulation_seed);

  Utils::fred_print_lap_time("RNG setup");
  Utils::fred_print_wall_time("\nFRED run %d started", Global::Simulation_run_number);

  // initializations

  // Initializes Synthetic Population location parameters
  Global::Places.get_parameters();

  // Loop over all locations and read in the household, schools and workplaces
  // and setup geographical layers
  Utils::fred_print_wall_time("\nFRED read_places started");
  Global::Places.read_all_places();
  Utils::fred_print_lap_time("Places.read_places");
  Utils::fred_print_wall_time("FRED read_places finished");

  // create visualization layer, if requested
  if(Global::Enable_Visualization_Layer) {
    Global::Visualization = new Visualization_Layer();
  }

  // create vector layer, if requested
  if(Global::Enable_Vector_Layer) {
    Global::Vectors = new Vector_Layer();
  }

  // initialize parameters and other static variables
  Demographics::initialize_static_variables();
  Activities::initialize_static_variables();
  Health::initialize_static_variables();
  Utils::fred_print_lap_time("initialize_static_variables");

  // setup visualuzation data directories
  if(Global::Enable_Visualization_Layer) {
    Global::Visualization->setup();
  }

  // finished setting up Conditions
  Global::Conditions.setup();
  Utils::fred_print_lap_time("Conditions.setup");

  // read in the population and have each person enroll
  // in each daily activity location identified in the population file
  Utils::fred_print_wall_time("\nFRED Pop.setup started");
  Global::Pop.setup();
  Utils::fred_print_wall_time("FRED Pop.setup finished");
  Utils::fred_print_lap_time("Pop.setup");
  Global::Places.setup_group_quarters();
  Utils::fred_print_lap_time("Places.setup_group_quarters");
  Global::Places.setup_households();
  Utils::fred_print_lap_time("Places.setup_households");

  // setup county lists of schools and workplaces.
  // this may optionally re-assigned students to in-state schools
  Global::Places.setup_counties();

  // define FRED-specific places and have each person enroll as needed

  // classrooms
  Global::Places.setup_classrooms();
  Global::Pop.assign_classrooms();
  Utils::fred_print_lap_time("assign classrooms");

  // offices
  Global::Places.setup_offices();
  Utils::fred_print_lap_time("setup_offices");
  Global::Pop.assign_offices();
  Utils::fred_print_lap_time("assign offices");

  if(Global::Enable_Hospitals) {
    Global::Places.assign_hospitals_to_households();
    Utils::fred_print_lap_time("assign hospitals to households");
  }

  // setup census_tract lists of schools and workplaces.
  Global::Places.setup_census_tracts();

  // after all enrollments, prepare to receive visitors
  Global::Places.prepare();
  Utils::fred_print_lap_time("place preparation");

  FRED_STATUS(0, "deleting place_label_map\n", "");
  Global::Places.delete_place_label_map();
  FRED_STATUS(0, "prepare places finished\n", "");
  
  // reassign workers (to schools, hospitals, groups quarters, etc)
  Global::Places.reassign_workers();
  Utils::fred_print_lap_time("reassign workers");

  if(Global::Enable_Vector_Layer) {
    Global::Vectors->setup();
    Utils::fred_print_lap_time("Vectors->setup");
  }

  // create networks if needed
  if(Global::Enable_Transmission_Network) {
    Global::Transmission_Network = new Network("Transmission_Network");
    //Global::Transmission_Network->test();
  }

  if(Global::Enable_Sexual_Partner_Network) {
    Global::Sexual_Partner_Network = new Sexual_Transmission_Network("Sexual_Partner_Network");
    Global::Sexual_Partner_Network->get_parameters();
    //Global::Sexual_Partner_Network->test();
  }

  // initialize sexual transmission network if needed
  if(Global::Enable_Sexual_Partner_Network) {   //mina
    Global::Sexual_Partner_Network->setup(argc, argv);
  }

  if(Global::Enable_Travel) {
    Utils::fred_print_wall_time("\nFRED Travel setup started");
    Global::Simulation_Region->set_population_size();
    Travel::setup(Global::Simulation_directory);
    Utils::fred_print_lap_time("Travel setup");
    Utils::fred_print_wall_time("FRED Travel setup finished");
  }

  if(Global::Quality_control) {
    Global::Pop.quality_control();
    Global::Places.quality_control();
    Global::Simulation_Region->quality_control();
    Global::Neighborhoods->quality_control();
    if(Global::Enable_Visualization_Layer) {
      Global::Visualization->quality_control();
    }
    if(Global::Enable_Vector_Layer) {
      Global::Vectors->quality_control();
      Global::Simulation_Region->set_population_size();
      Global::Vectors->swap_county_people();
    }
    if(Global::Track_network_stats) {
      Global::Pop.get_network_stats(Global::Simulation_directory);
    }
    Utils::fred_print_lap_time("quality control");
  }

  /*
    if(Global::Track_age_distribution) {
    Global::Pop.print_age_distribution(Global::Simulation_directory,
    (char *) Global::Sim_Start_Date->get_YYYYMMDD().c_str(),
    Global::Simulation_run_number);
    }
  */

  if(Global::Enable_Seasonality) {
    Global::Clim->print_summary();
  }

  if(Global::Report_Mean_Household_Income_Per_School) {
    Global::Pop.report_mean_hh_income_per_school();
  }

  if(Global::Report_Mean_Household_Size_Per_School) {
    Global::Pop.report_mean_hh_size_per_school();
  }

  if(Global::Report_Mean_Household_Distance_From_School) {
    Global::Pop.report_mean_hh_distance_from_school();
  }

  if(Global::Report_Childhood_Presenteeism) {
    Global::Pop.set_school_income_levels();
    Global::Places.setup_school_income_quartile_pop_sizes();
    Global::Places.setup_household_childcare();
  }

  if(Global::Report_Mean_Household_Stats_Per_Income_Category &&
     Global::Report_Epidemic_Data_By_Census_Tract) {
    Global::Income_Category_Tracker = new Tracker<int>("Income Category Tracker","income_cat");
    Global::Tract_Tracker = new Tracker<long int>("Census Tract Tracker","Tract");
    Global::Pop.report_mean_hh_stats_per_income_category_per_census_tract();
  } else if(Global::Report_Mean_Household_Stats_Per_Income_Category) {
    Global::Income_Category_Tracker = new Tracker<int>("Income Category Tracker","income_cat");
    Global::Pop.report_mean_hh_stats_per_income_category();
  } else if(Global::Report_Epidemic_Data_By_Census_Tract) {
    Global::Tract_Tracker = new Tracker<long int>("Census Tract Tracker","Tract");
    Global::Pop.report_mean_hh_stats_per_census_tract();
  }
  
  // Global tracker allows us to have as many variables we
  // want from wherever in the output file
  Global::Daily_Tracker = new Tracker<int>("Main Daily Tracker","Day");
  
  // prepare conditions after population is all set up
  FRED_VERBOSE(0, "prepare conditions\n");
  Global::Conditions.prepare_conditions();
  Utils::fred_print_lap_time("prepare_conditions");

  if(Global::Enable_Vector_Layer) {
    Global::Vectors->init_prior_immunity_by_county();
    for(int d = 0; d < Global::Conditions.get_number_of_conditions(); ++d) {
      Global::Vectors->init_prior_immunity_by_county(d);
    }
    Utils::fred_print_lap_time("vector_layer_initialization");
  }

  // initialize generic activities
  Activities::before_run();

  Utils::fred_print_wall_time("FRED initialization complete");

  Utils::fred_start_timer(&Global::Simulation_start_time);
  Utils::fred_print_initialization_timer();

  // unit test for Logit
  if (0&&Global::Test) {
    Logit::test();
    exit(0);
  }
}


void fred_step(int day) {

  Utils::fred_start_day_timer();

  // optional: reseed the random number generator to create alternative
  // simulation from a given initial point
  if(day == Global::Reseed_day) {
    fprintf(Global::Statusfp, "************** reseed day = %d\n", day);
    fflush(Global::Statusfp);
    Random::set_seed(Global::Simulation_seed + Global::Simulation_run_number - 1);
  }

  // optional: periodically output distributions of the population demographics
  /*
  if(Global::Track_age_distribution) {
    if(Date::get_month() == 1 && Date::get_day_of_month() == 1) {
      char date_string[80];
      strcpy(date_string, (char *) Global::Sim_Current_Date->get_YYYYMMDD().c_str());
      Global::Pop.print_age_distribution(Global::Simulation_directory, date_string, Global::Simulation_run_number);
      Global::Places.print_household_size_distribution(Global::Simulation_directory, date_string, Global::Simulation_run_number);
    }
  }
  */

  // reset lists of infectious, susceptibles; update vector population, if any
  Global::Places.update(day);
  Utils::fred_print_lap_time("day %d update places", day);

  // update population demographics
  Global::Pop.update_demographics(day);
  Utils::fred_print_lap_time("day %d update demographics", day);

  // update population mobility and stage-of-life activities
  Global::Places.update_population_dynamics(day);
  Utils::fred_print_lap_time("day %d update population dynamics", day);

  // remove dead from population
  Global::Pop.remove_dead_from_population(day);
  Utils::fred_print_lap_time("day %d remove dead from population", day);

  // remove migrants from population
  Global::Pop.remove_migrants_from_population(day);

  // update vector populations
  if(Global::Enable_Vector_Layer) {
    Global::Vectors->update(day);
  }

  // update travel decisions
  Travel::update_travel(day);

  // update generic activities (individual activities updated only if
  // needed -- see below)
  Activities::update(day);

  if(Global::Enable_Sexual_Partner_Network) {
    //mina  update network every day **********************
    Global::Sexual_Partner_Network->update(day);
    //mina  END **********************
  }

  // shuffle the order of conditions to reduce systematic bias
  vector<int> order;
  order.clear();
  for(int d = 0; d < Global::Conditions.get_number_of_conditions(); ++d) {
    order.push_back(d);
  }
  if(Global::Conditions.get_number_of_conditions() > 1) {
    FYShuffle<int>(order);
  }

  // update epidemic for each condition in turn
  for(int d = 0; d < Global::Conditions.get_number_of_conditions(); ++d) {
    int condition_id = order[d];
    Condition* condition = Global::Conditions.get_condition(condition_id);
    condition->update(day);
    Utils::fred_print_lap_time("day %d update epidemic for condition %d", day, condition_id);
  }

  // print daily reports and visualization data
  for(int d = 0; d < Global::Conditions.get_number_of_conditions(); ++d) {
    Global::Conditions.get_condition(d)->report(day);
  }
  Utils::fred_print_lap_time("day %d report conditions", day);

  Global::Pop.report(day);
  Utils::fred_print_lap_time("day %d report population", day);

  if(Global::Report_Presenteeism || Global::Report_Childhood_Presenteeism) {
    Activities::report(day);
  }

  // print visualization data if desired
  if(Global::Enable_Visualization_Layer) {
    Global::Visualization->print_visualization_data(day);
    Utils::fred_print_lap_time("day %d print_visualization_data", day);
  }

  // optional: report change in demographics at end of each year
  if(Global::Enable_Population_Dynamics && Global::Verbose
     && Date::get_month() == 12 && Date::get_day_of_month() == 31) {
    Global::Pop.quality_control();
  }

  // optional: report County demographics at end of each year
  if(Global::Report_County_Demographic_Information && Date::get_month() == 12 && Date::get_day_of_month() == 31) {
    Global::Places.report_county_populations();
  }

#pragma omp parallel sections
  {
#pragma omp section
    {
      // flush infections file buffer
      fflush(Global::Infectionfp);
    }
  }

  // print daily reports
  Utils::fred_print_resource_usage(day);
  Utils::fred_print_wall_time("day %d finished", day);
  FRED_STATUS(0, "%s ", Date::get_date_string().c_str());
  Utils::fred_print_day_timer(day);
  Global::Daily_Tracker->output_inline_report_format_for_index(day, Global::Outfp);

  // advance date counter
  Date::update();
}


void make_output_variable_files() {
  // parse the output files into csv files, one for each column
  char dir[FRED_STRING_SIZE];
  sprintf(dir, "%s/RUN%d/DAILY",
	  Global::Simulation_directory,
	  Global::Simulation_run_number);
  Utils::fred_make_directory(dir);
  char outfile[FRED_STRING_SIZE];
  sprintf(outfile, "%s/RUN%d/out.csv",
	  Global::Simulation_directory,
	  Global::Simulation_run_number);
  FILE *fp = fopen(outfile, "r");
  if (fp == NULL) {
    Utils::fred_abort("Fred: can not find file %s\n", outfile);
  }

  // determine number of columns in the out file
  char line[100*FRED_STRING_SIZE];
  int n = 0;
  if (fgets(line,100*FRED_STRING_SIZE,fp)!=NULL) {
    char * pch;
    pch = strtok (line,",\n");
    while (pch != NULL) {
      // printf("column name %d = |%s|\n", n, pch);
      n++;
      pch = strtok (NULL, ",\n");
    }
  }
  fclose(fp);

  // produce a file for each column
  for (int col = 0; col < n; col++) {
    FILE *fp = fopen(outfile, "r");
    if (fp == NULL) {
      Utils::fred_abort("Fred: can not find file %s\n", outfile);
    }
    // get name of file
    char *name;
    char filename[FRED_STRING_SIZE];
    char line[100*FRED_STRING_SIZE];
    if (fgets(line,100*FRED_STRING_SIZE,fp)!=NULL) {
      int i = 0;
      name = strtok (line,",\n");
      while (i < col && name != NULL) {
	name = strtok (NULL, ",\n");
	i++;
      }
      if (name != NULL) {
	if (strcmp(name, "Day") == 0) {
	  continue;
	}
	sprintf(filename, "%s/%s.txt", dir, name);
	// printf("col filename %d = |%s|\n", col, filename);
	FILE *colfp = fopen(filename, "w");
	if (colfp == NULL) {
	  Utils::fred_abort("Can't write to file %s\n", filename);
	}
	int day = 0;
	while (fgets(line,100*FRED_STRING_SIZE,fp)!=NULL) {
	  char* value;
	  int j = 0;
	  value = strtok(line,",\n");
	  while (j < col && name != NULL) {
	    value = strtok (NULL, ",\n");
	    j++;
	  }
	  fprintf(colfp, "%d %s\n", day, value);
	  day++;
	}
	fclose(colfp);
      }
      else {
	Utils::fred_abort("Bad column %d name\n", col);
      }
    }
    else {
      Utils::fred_abort("Bad outfile\n");
    }
    fclose(fp);
  }

}


void fred_finish() {
  //Global::Daily_Tracker->create_full_log(10,cout);
  fflush(Global::Infectionfp);
  
  // final reports
  if(Global::Report_Mean_Household_Stats_Per_Income_Category && Global::Report_Epidemic_Data_By_Census_Tract) {
    Global::Tract_Tracker->output_csv_report_format(Global::Tractfp);
    Global::Income_Category_Tracker->output_csv_report_format(Global::IncomeCatfp);
  } else if(Global::Report_Mean_Household_Stats_Per_Income_Category)  {
    Global::Income_Category_Tracker->output_csv_report_format(Global::IncomeCatfp);
  } else if(Global::Report_Epidemic_Data_By_Census_Tract) {
    Global::Tract_Tracker->output_csv_report_format(Global::Tractfp);
  }
  Activities::end_of_run();

  // report timing info
  Utils::fred_print_lap_time(&Global::Simulation_start_time,
			     "\nFRED simulation complete. Excluding initialization, %d days",
			     Global::Days);
  Utils::fred_print_wall_time("FRED finished");
  Utils::fred_print_finish_timer();

  Global::Pop.end_of_run();
  Global::Places.end_of_run();
  Global::Conditions.end_of_run();

  if(Global::Enable_Transmission_Network) {
    // Global::Transmission_Network->print();
  }

  if(Global::Enable_Sexual_Partner_Network) {
    // Global::Sexual_Partner_Network->print();
  }

  // close all open output files with global file pointers
  Utils::fred_end();

  // create files for plotting
  make_output_variable_files();

}

