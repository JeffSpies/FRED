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

void Place_List::report_school_distributions(int day) {
  // original size distribution
  int count[21];
  int osize[21];
  int nsize[21];
  // size distribution of schools
  for(int c = 0; c <= 20; ++c) {
    count[c] = 0;
    osize[c] = 0;
    nsize[c] = 0;
  }
  for(int p = 0; p < get_number_of_schools(); ++p) {
    int os = get_school(p)->get_orig_size();
    int ns = get_school(p)->get_size();
    int n = os / 50;
    if(n > 20) {
      n = 20;
    }
    count[n]++;
    osize[n] += os;
    nsize[n] += ns;
  }
  fprintf(Global::Statusfp, "SCHOOL SIZE distribution: ");
  for(int c = 0; c <= 20; ++c) {
    fprintf(Global::Statusfp, "%d %d %0.2f %0.2f | ",
	    c, count[c], count[c] ? (1.0 * osize[c]) / (1.0 * count[c]) : 0, count[c] ? (1.0 * nsize[c]) / (1.0 * count[c]) : 0);
  }
  fprintf(Global::Statusfp, "\n");

  return;

  int year = day / 365;
  char filename[FRED_STRING_SIZE];
  FILE* fp = NULL;

  sprintf(filename, "%s/school-%d.txt", Global::Simulation_directory, year);
  fp = fopen(filename, "w");
  for(int p = 0; p < get_number_of_schools(); ++p) {
    Place* h = get_school(p);
    fprintf(fp, "%s orig_size %d current_size %d\n", h->get_label(), h->get_orig_size()-h->get_staff_size(), h->get_size()-h->get_staff_size());
    /*
      School * s = static_cast<School*>(h);
      for(int a = 1; a <=20; ++a) {
      if(s->get_orig_students_in_grade(a) > 0) {
      fprintf(fp, "SCHOOL %s age %d orig %d current %d\n",
      s->get_label(), a, s->get_orig_students_in_grade(a), s->get_students_in_grade(a));
      }
      }
      fprintf(fp,"\n");
    */
  }
  fclose(fp);

  sprintf(filename, "%s/work-%d.txt", Global::Simulation_directory, year);
  fp = fopen(filename, "w");
  for(int p = 0; p < this->workplaces.size(); ++p) {
    Place* h = get_workplace(p);
    fprintf(fp, "%s orig_size %d current_size %d\n", h->get_label(), h->get_orig_size(), h->get_size());
  }
  fclose(fp);

  sprintf(filename, "%s/house-%d.txt", Global::Simulation_directory, year);
  fp = fopen(filename, "w");
  for(int p = 0; p < get_number_of_households(); ++p) {
    Place* h = get_household(p);
    fprintf(fp, "%s orig_size %d current_size %d\n", h->get_label(), h->get_orig_size(), h->get_size());
  }
  fclose(fp);
}

void Place_List::report_household_distributions() {
  int number_of_households = this->households.size();
  if(Global::Verbose) {
    int count[20];
    int total = 0;
    // size distribution of households
    for(int c = 0; c <= 10; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      int n = get_household(p)->get_size();
      if(n <= 10) {
	count[n]++;
      } else {
	count[10]++;
      }
      total++;
    }
    fprintf(Global::Statusfp, "Household size distribution: N = %d ", total);
    for(int c = 0; c <= 10; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%) ",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
    
    // original size distribution
    int hsize[20];
    total = 0;
    // size distribution of households
    for(int c = 0; c <= 10; ++c) {
      count[c] = 0;
      hsize[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      int n = get_household(p)->get_orig_size();
      int hs = get_household(p)->get_size();
      if(n <= 10) {
	count[n]++;
	hsize[n] += hs;
      } else {
	count[10]++;
	hsize[10] += hs;
      }
      total++;
    }
    fprintf(Global::Statusfp, "Household orig distribution: N = %d ", total);
    for(int c = 0; c <= 10; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%) %0.2f ",
	      c, count[c], (100.0 * count[c]) / total, count[c] ? ((double)hsize[c] / (double)count[c]) : 0.0);
    }
    fprintf(Global::Statusfp, "\n");
  }

  return;

  if(Global::Verbose) {
    int count[100];
    int total = 0;
    // age distribution of heads of households
    for(int c = 0; c < 100; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      Person* per = NULL;
      for(int i = 0; i < h->get_size(); ++i) {
	if(h->get_enrollee(i)->is_householder()) {
	  per = h->get_enrollee(i);
	}
      }
      if(per == NULL) {
	FRED_WARNING("Help! No head of household found for household id %d label %s groupquarters: %d\n",
		     h->get_id(), h->get_label(), h->is_group_quarters()?1:0);
	count[0]++;
      } else {
	int a = per->get_age();
	if(a < 100) {
	  count[a]++;
	} else {
	  count[99]++;
	}
	total++;
      }
    }
    fprintf(Global::Statusfp, "\nAge distribution of heads of households: %d households\n", total);
    for(int c = 0; c < 100; ++c) {
      fprintf(Global::Statusfp, "age %2d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[20];
    int total = 0;
    // adult distribution of households
    for (int c = 0; c < 15; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      int n = h->get_adults();
      if(n < 15) {
	count[n]++;
      } else {
	count[14]++;
      }
      total++;
    }
    fprintf(Global::Statusfp, "\nHousehold adult size distribution: %d households\n", total);
    for(int c = 0; c < 15; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[20];
    int total = 0;
    // children distribution of households
    for(int c = 0; c < 15; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      int n = h->get_children();
      if(n < 15) {
	count[n]++;
      } else {
	count[14]++;
      }
      total++;
    }
    fprintf(Global::Statusfp, "\nHousehold children size distribution: %d households\n", total);
    for (int c = 0; c < 15; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[20];
    int total = 0;
    // adult distribution of households with children
    for(int c = 0; c < 15; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      if(h->get_children() == 0) {
	continue;
      }
      int n = h->get_adults();
      if(n < 15) {
	count[n]++;
      } else {
	count[14]++;
      }
      total++;
    }
    fprintf(Global::Statusfp, "\nHousehold w/ children, adult size distribution: %d households\n", total);
    for(int c = 0; c < 15; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  /*
  // relationship between children and decision maker
  if(Global::Verbose > 1 && Global::Enable_Behaviors) {
    // find adult decision maker for each child
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      if(h->get_children() == 0) {
	continue;
      }
      int size = h->get_size();
      for(int i = 0; i < size; ++i) {
	Person* child = h->get_enrollee(i);
	int ch_age = child->get_age();
	if (ch_age < 18) {
	  int ch_rel = child->get_relationship();
	  Person* dm = child->get_health_decision_maker();
	  if(dm == NULL) {
	    printf("DECISION_MAKER: household %d %s  child: %d %d is making own health decisions\n",
		   h->get_id(), h->get_label(), ch_age, ch_rel);
	  } else {
	    int dm_age = dm->get_age();
	    int dm_rel = dm->get_relationship();
	    if(dm_rel != 1 || ch_rel != 3) {
	      printf("DECISION_MAKER: household %d %s  decision_maker: %d %d child: %d %d\n",
		     h->get_id(), h->get_label(), dm_age, dm_rel, ch_age, ch_rel);
	    }
	  }
        }
      }
    }
  }
  */

  if(Global::Verbose) {
    int count[100];
    int total = 0;
    // age distribution of heads of households
    for(int c = 0; c < 100; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      Person* per = NULL;
      for(int i = 0; i < h->get_size(); ++i) {
	if(h->get_enrollee(i)->is_householder()) {
	  per = h->get_enrollee(i);
	}
      }
      if(per == NULL) {
	FRED_WARNING("Help! No head of household found for household id %d label %s groupquarters: %d\n",
		     h->get_id(), h->get_label(), h->is_group_quarters() ? 1 : 0);
	count[0]++;
      } else {
	int a = per->get_age();
	if(a < 100) {
	  count[a]++;
	} else {
	  count[99]++;
	}
	total++;
      }
    }
    fprintf(Global::Statusfp, "\nAge distribution of heads of households: %d households\n", total);
    for(int c = 0; c < 100; ++c) {
      fprintf(Global::Statusfp, "age %2d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[100];
    int total = 0;
    int children = 0;
    // age distribution of heads of households with children
    for(int c = 0; c < 100; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      Person* per = NULL;
      if(h->get_children() == 0){
	continue;
      }
      children += h->get_children();
      for(int i = 0; i < h->get_size(); ++i) {
	if(h->get_enrollee(i)->is_householder()) {
	  per = h->get_enrollee(i);
	}
      }
      if(per == NULL) {
	FRED_WARNING("Help! No head of household found for household id %d label %s groupquarters: %d\n",
		     h->get_id(), h->get_label(), h->is_group_quarters()?1:0);
	count[0]++;
      } else {
	int a = per->get_age();
	if(a < 100) {
	  count[a]++;
	} else {
	  count[99]++;
	}
	total++;
      }
    }
    fprintf(Global::Statusfp, "\nAge distribution of heads of households with children: %d households\n", total);
    for(int c = 0; c < 100; ++c) {
      fprintf(Global::Statusfp, "age %2d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "children = %d\n", children);
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count_has_school_age = 0;
    int count_has_school_age_and_unemployed_adult = 0;
    int total_hh = 0;

    //Households with school-age children and at least one unemployed adult
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      Person* per = NULL;
      total_hh++;
      if(h->get_children() == 0) {
	continue;
      }
      if(h->has_school_aged_child()) {
	count_has_school_age++;
      }
      if(h->has_school_aged_child_and_unemployed_adult()) {
	count_has_school_age_and_unemployed_adult++;
      }
    }
    fprintf(Global::Statusfp, "\nHouseholds with school-aged children and at least one unemployed adult\n");
    fprintf(Global::Statusfp, "Total Households: %d\n", total_hh);
    fprintf(Global::Statusfp, "Total Households with school-age children: %d\n", count_has_school_age);
    fprintf(Global::Statusfp, "Total Households with school-age children and at least one unemployed adult: %d\n", count_has_school_age_and_unemployed_adult);
  }

}


void Place_List::quality_control() {
  //Can't do the quality control until all of the population files have been read
  assert(Global::Pop.is_load_completed());

  int number_of_households = get_number_of_households();
  int number_of_schools = get_number_of_schools();
  int number_of_neighborhoods = get_number_of_neighborhoods();
  int number_of_workplaces = get_number_of_workplaces();

  FRED_STATUS(0, "places quality control check for places\n");

  if(Global::Verbose) {
    int hn = 0;
    int nn = 0;
    int sn = 0;
    int wn = 0;
    double hsize = 0.0;
    double nsize = 0.0;
    double ssize = 0.0;
    double wsize = 0.0;
    // mean size by place type

    for(int p = 0; p < number_of_households; ++p) {
      int n = get_household(p)->get_size();
      hn++;
      hsize += n;
    }

    for(int p = 0; p < number_of_neighborhoods; ++p) {
      int n = get_neighborhood(p)->get_size();
      nn++;
      nsize += n;
    }

    for(int p = 0; p < number_of_schools; ++p) {
      int n = get_school(p)->get_size();
      sn++;
      ssize += n;
    }

    for(int p = 0; p < number_of_workplaces; ++p) {
      int n = get_workplace(p)->get_size();
      wn++;
      wsize += n;
    }

    if(hn) {
      hsize /= hn;
    }
    if(nn) {
      nsize /= nn;
    }
    if(sn) {
      ssize /= sn;
    }
    if(wn) {
      wsize /= wn;
    }
    fprintf(Global::Statusfp, "\nMEAN PLACE SIZE: H %.2f N %.2f S %.2f W %.2f\n",
	    hsize, nsize, ssize, wsize);
  }

  if(Global::Verbose>1) {
    char filename[FRED_STRING_SIZE];
    sprintf(filename, "%s/houses.dat", Global::Simulation_directory);
    FILE* fp = fopen(filename, "w");
    for(int p = 0; p < number_of_households; p++) {
      Place* h = get_household(p);
      double x = Geo::get_x(h->get_longitude());
      double y = Geo::get_y(h->get_latitude());
      fprintf(fp, "%f %f\n", x, y);
    }
    fclose(fp);
  }

  // household type
  int htypes = 21;
  enum htype_t {
    SINGLE_FEMALE,
    SINGLE_MALE,
    OPP_SEX_SIM_AGE_PAIR,
    OPP_SEX_DIF_AGE_PAIR,
    OPP_SEX_TWO_PARENTS,
    SINGLE_PARENT,
    SINGLE_PAR_MULTI_GEN_FAMILY,
    TWO_PAR_MULTI_GEN_FAMILY,
    UNATTENDED_KIDS,
    OTHER_FAMILY,
    YOUNG_ROOMIES,
    OLDER_ROOMIES,
    MIXED_ROOMIES,
    SAME_SEX_SIM_AGE_PAIR,
    SAME_SEX_DIF_AGE_PAIR,
    SAME_SEX_TWO_PARENTS,
    DORM_MATES,
    CELL_MATES,
    BARRACK_MATES,
    NURSING_HOME_MATES,
    UNKNOWN,
  };

  string htype[21] = {
    "single-female",
    "single-male",
    "opp-sex-sim-age-pair",
    "opp-sex-dif-age-pair",
    "opp-sex-two-parent-family",
    "single-parent-family",
    "single-parent-multigen-family",
    "two-parent-multigen-family",
    "unattended-minors",
    "other-family",
    "young-roomies",
    "older-roomies",
    "mixed-roomies",
    "same-sex-sim-age-pair",
    "same-sex-dif-age-pair",
    "same-sex-two-parent-family",
    "dorm-mates",
    "cell-mates",
    "barrack-mates",
    "nursing-home_mates",
    "unknown",
  };

  int type[htypes];
  int ttotal[htypes];
  int hnum = 0;
  for (int i = 0; i < htypes; i++) { type[i] = 0; ttotal[i] = 0;}

  for(int p = 0; p < number_of_households; ++p) {
    Person* per = NULL;
    hnum++;
    Household* h = get_household(p);
    int t = h->get_orig_household_structure();
    type[t]++;
    ttotal[t] += h->get_size();
  }

  printf("HOUSEHOLD_TYPE DISTRIBUTION\n");
  for(int t = 0; t < htypes; ++t) {
    printf("HOUSEHOLD TYPE DIST: ");
    printf("%30s: %8d households (%5.1f%%) with %8d residents (%5.1f%%)\n",
	   htype[t].c_str(), type[t],
	   (100.0*type[t])/hnum, ttotal[t],
	   (100.0*ttotal[t]/Global::Pop.get_population_size()));
  }

  if (0) {
    FILE* hfp = fopen("households.txt", "w");
    FILE* pfp = fopen("people.txt", "w");
    int pi = 0;
    for(int p = 0; p < number_of_households; ++p) {
      Household* house = get_household(p);
      fprintf(hfp, "hid %d household %s size %d htype %s\n", 
	      p, house->get_label(), house->get_size(), house->get_orig_household_structure_label());
      int hsize = house->get_size();
      for(int j = 0; j < hsize; ++j) {
	Person* person = house->get_enrollee(j);
	fprintf(pfp, "pid %d person %d age %d sex %c household %s htype %s\n",
		pi++, person->get_id(), person->get_age(), person->get_sex(),
		person->get_household()->get_label(),
		person->get_household_structure_label());
      }
    }
    fclose(hfp);
    fclose(pfp);
  }

  if(Global::Verbose) {
    int count[20];
    int total = 0;
    // size distribution of households
    for(int c = 0; c < 15; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      int n = get_household(p)->get_size();
      if(n < 15) {
	count[n]++;
      } else {
	count[14]++;
      }
      total++;
    }
    fprintf(Global::Statusfp, "\nHousehold size distribution: %d households\n", total);
    for(int c = 0; c < 15; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[20];
    int total = 0;
    // adult distribution of households
    for(int c = 0; c < 15; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      int n = h->get_adults();
      if(n < 15) {
	count[n]++;
      } else {
	count[14]++;
      }
      total++;
    }
    fprintf(Global::Statusfp, "\nHousehold adult size distribution: %d households\n", total);
    for(int c = 0; c < 15; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[20];
    int total = 0;
    // children distribution of households
    for (int c = 0; c < 15; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      int n = h->get_children();
      if(n < 15) {
	count[n]++;
      } else {
	count[14]++;
      }
      total++;
    }
    fprintf(Global::Statusfp, "\nHousehold children size distribution: %d households\n", total);
    for(int c = 0; c < 15; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[20];
    int total = 0;
    // adult distribution of households with children
    for(int c = 0; c < 15; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      if(h->get_children() == 0) {
	continue;
      }
      int n = h->get_adults();
      if(n < 15) {
	count[n]++;
      } else {
	count[14]++;
      }
      total++;
    }
    fprintf(Global::Statusfp, "\nHousehold w/ children, adult size distribution: %d households\n", total);
    for(int c = 0; c < 15; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  /*
  // relationship between children and decision maker
  if(Global::Verbose > 1 && Global::Enable_Behaviors) {
    // find adult decision maker for each child
    for(int p = 0; p < number_of_households; ++p) {
      Household* h = get_household(p);
      if(h->get_children() == 0) {
	continue;
      }
      int size = h->get_size();
      for(int i = 0; i < size; ++i) {
	Person* child = h->get_enrollee(i);
	int ch_age = child->get_age();
	if(ch_age < 18) {
	  int ch_rel = child->get_relationship();
	  Person* dm = child->get_health_decision_maker();
	  if(dm == NULL) {
	    printf("DECISION_MAKER: household %d %s  child: %d %d is making own health decisions\n",
		   h->get_id(), h->get_label(), ch_age, ch_rel);
	  } else {
	    int dm_age = dm->get_age();
	    int dm_rel = dm->get_relationship();
	    if(dm_rel != 1 || ch_rel != 3) {
	      printf("DECISION_MAKER: household %d %s  decision_maker: %d %d child: %d %d\n",
		     h->get_id(), h->get_label(), dm_age, dm_rel, ch_age, ch_rel);
	    }
	  }
	}
      }
    }
  }
  */

  if(Global::Verbose) {
    int count[100];
    int total = 0;
    // age distribution of heads of households
    for(int c = 0; c < 100; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Person* per = NULL;
      Household* h = get_household(p);
      for(int i = 0; i < h->get_size(); ++i) {
	if(h->get_enrollee(i)->is_householder()) {
	  per = h->get_enrollee(i);
	}
      }
      if(per == NULL) {
	FRED_WARNING("Help! No head of household found for household id %d label %s size %d groupquarters: %d\n",
		     h->get_id(), h->get_label(), h->get_size(), h->is_group_quarters() ? 1 : 0);
	count[0]++;
      } else {
	int a = per->get_age();
	if(a < 100) {
	  count[a]++;
	} else {
	  count[99]++;
	}
	total++;
      }
    }
    fprintf(Global::Statusfp, "\nAge distribution of heads of households: %d households\n", total);
    for(int c = 0; c < 100; ++c) {
      fprintf(Global::Statusfp, "age %2d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[100];
    int total = 0;
    int children = 0;
    // age distribution of heads of households with children
    for(int c = 0; c < 100; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_households; ++p) {
      Person* per = NULL;
      Household* h = get_household(p);
      if(h->get_children() == 0) {
	continue;
      }
      children += h->get_children();
      for(int i = 0; i < h->get_size(); ++i) {
	if(h->get_enrollee(i)->is_householder()) {
	  per = h->get_enrollee(i);
	}
      }
      if(per == NULL) {
	FRED_WARNING("Help! No head of household found for household id %d label %s groupquarters: %d\n",
		     h->get_id(), h->get_label(), h->is_group_quarters() ? 1 : 0);
	count[0]++;
      } else {
	int a = per->get_age();
	if(a < 100) {
	  count[a]++;
	} else {
	  count[99]++;
	}
	total++;
      }
    }
    fprintf(Global::Statusfp, "\nAge distribution of heads of households with children: %d households\n", total);
    for(int c = 0; c < 100; ++c) {
      fprintf(Global::Statusfp, "age %2d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "children = %d\n", children);
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count_has_school_age = 0;
    int count_has_school_age_and_unemployed_adult = 0;
    int total_hh = 0;

    //Households with school-age children and at least one unemployed adult
    for(int p = 0; p < number_of_households; ++p) {
      Person* per = NULL;
      Household* h = get_household(p);
      total_hh++;
      if(h->get_children() == 0) {
	continue;
      }
      if(h->has_school_aged_child()) {
	count_has_school_age++;
      }
      if(h->has_school_aged_child_and_unemployed_adult()) {
	count_has_school_age_and_unemployed_adult++;
      }
    }
    fprintf(Global::Statusfp, "\nHouseholds with school-aged children and at least one unemployed adult\n");
    fprintf(Global::Statusfp, "Total Households: %d\n", total_hh);
    fprintf(Global::Statusfp, "Total Households with school-age children: %d\n", count_has_school_age);
    fprintf(Global::Statusfp, "Total Households with school-age children and at least one unemployed adult: %d\n", count_has_school_age_and_unemployed_adult);
  }

  if(Global::Verbose) {
    int count[100];
    int total = 0;
    // size distribution of schools
    for(int c = 0; c < 20; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_schools; ++p) {
      int s = get_school(p)->get_size();
      int n = s / 50;
      if(n < 20) {
	count[n]++;
      } else {
	count[19]++;
      }
      total++;
    }
    fprintf(Global::Statusfp, "\nSchool size distribution: %d schools\n", total);
    for(int c = 0; c < 20; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      (c+1)*50, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    // age distribution in schools
    fprintf(Global::Statusfp, "\nSchool age distribution:\n");
    int count[100];
    for(int c = 0; c < 100; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_schools; ++p) {
      for(int c = 0; c < 100; ++c) {
	count[c] += get_school(p)->get_students_in_grade(c);
      }
    }
    for(int c = 0; c < 100; ++c) {
      fprintf(Global::Statusfp, "age = %2d  students = %6d\n",
	      c, count[c]);;
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[50];
    int total = 0;
    // size distribution of classrooms
    for(int c = 0; c < 50; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_schools; ++p) {
      School* school = get_school(p);
      for(int c = 0; c < school->get_number_of_classrooms(); ++c) {
        int s = school->get_classroom(c)->get_size();
        int n = s;
        if(n < 50) {
          count[n]++;
        } else {
          count[50-1]++;
        }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nClassroom size distribution: %d classrooms\n", total);
    for(int c = 0; c < 50; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if(Global::Verbose) {
    int count[101];
    int small_employees = 0;
    int med_employees = 0;
    int large_employees = 0;
    int xlarge_employees = 0;
    int total_employees = 0;
    int total = 0;
    // size distribution of workplaces
    for(int c = 0; c <= 100; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_workplaces; ++p) {
      int s = get_workplace(p)->get_size();
      if(s <= 100) {
	count[s]++;
      } else {
	count[100]++;
      }
      if(s < 50) {
	small_employees += s;
      } else if (s < 100) {
	med_employees += s;
      } else if (s < 500) {
	large_employees += s;
      } else {
	xlarge_employees += s;
      }
      total_employees += s;
      total++;
    }
    for(int p = 0; p < number_of_schools; ++p) {
      int s = get_school(p)->get_staff_size();
      if(s <= 100) {
	count[s]++;
      } else {
	count[100]++;
      }
      if(s < 50) {
	small_employees += s;
      } else if (s < 100) {
	med_employees += s;
      } else if (s < 500) {
	large_employees += s;
      } else {
	xlarge_employees += s;
      }
      total_employees += s;
      total++;
    }
    fprintf(Global::Statusfp, "\nWorkplace size distribution: %d workplaces\n", total);
    for(int c = 0; c <= 100; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      (c+1)*1, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n\n");

    fprintf(Global::Statusfp, "employees at small workplaces (1-49): ");
    fprintf(Global::Statusfp, "%d\n", small_employees);

    fprintf(Global::Statusfp, "employees at medium workplaces (50-99): ");
    fprintf(Global::Statusfp, "%d\n", med_employees);

    fprintf(Global::Statusfp, "employees at large workplaces (100-499): ");
    fprintf(Global::Statusfp, "%d\n", large_employees);

    fprintf(Global::Statusfp, "employees at xlarge workplaces (500-up): ");
    fprintf(Global::Statusfp, "%d\n", xlarge_employees);

    fprintf(Global::Statusfp, "total employees: %d\n\n", total_employees);
  }

  /*
    if(Global::Verbose) {
    int covered[4];
    int all[4];
    // distribution of sick leave in workplaces
    for(int c = 0; c < 4; ++c) { all[c] = covered[c] = 0; }
    for(int p = 0; p < number_of_places; ++p) {
    Workplace* work = get_workplace(p);
    char s = work->get_size_code();
    bool sl = work->is_sick_leave_available();
    switch(s) {
    case 'S':
    all[0] += s;
    if(sl) {
    covered[0] += s;
    }
    break;
    case 'M':
    all[1] += s;
    if(sl) {
    covered[1] += s;
    }
    break;
    case 'L':
    all[2] += s;
    if(sl) {
    covered[2] += s;
    }
    break;
    case 'X':
    all[3] += s;
    if(sl) {
    covered[3] += s;
    }
    break;
    }
    }
    fprintf(Global::Statusfp, "\nWorkplace sick leave coverage: ");
    for(int c = 0; c < 4; ++c) {
    fprintf(Global::Statusfp, "%3d: %d/%d %5.2f | ",
    c, covered[c], all[c], (all[c]? (1.0*covered[c])/all[c] : 0));
    }
    fprintf(Global::Statusfp, "\n");
    }
  */

  if(Global::Verbose) {
    int count[60];
    int total = 0;
    // size distribution of offices
    for(int c = 0; c < 60; ++c) {
      count[c] = 0;
    }
    for(int p = 0; p < number_of_workplaces; ++p) {
      Workplace* w = get_workplace(p);
      for(int off = 0; off < w->get_number_of_offices(); ++off) {
        int s = w->get_office(off)->get_size();
        int n = s;
        if(n < 60) {
          count[n]++;
        } else {
          count[60 - 1]++;
        }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nOffice size distribution: %d offices\n", total);
    for(int c = 0; c < 60; ++c) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
	      c, count[c], (100.0 * count[c]) / total);
    }
    fprintf(Global::Statusfp, "\n");
  }
  if(Global::Verbose) {
    fprintf(Global::Statusfp, "places quality control finished\n");
    fflush(Global::Statusfp);
  }
}

