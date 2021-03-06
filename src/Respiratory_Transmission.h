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
// File: Respiratory_Transmission.h
//

#ifndef _FRED_RESPIRATORY_TRANSMISSION_H
#define _FRED_RESPIRATORY_TRANSMISSION_H

#include "Transmission.h"
class Condition;
class Mixing_Group;
class Person;
class Place;

class Respiratory_Transmission : public Transmission {

public:
  
  Respiratory_Transmission();
  ~Respiratory_Transmission();
  void setup(Condition* condition);
  void spread_infection(int day, int condition_id, Mixing_Group* mixing_group);
  void spread_infection(int day, int condition_id, Place* place);

private:

  // place-specific transmission mode parameters
  bool enable_neighborhood_density_transmission;
  bool enable_density_transmission_maximum_infectees;
  int density_transmission_maximum_infectees;
  double** prob_contact;

  void default_transmission_model(int day, int condition_id, Place* place);
  void age_based_transmission_model(int day, int condition_id, Place* place);
  void pairwise_transmission_model(int day, int condition_id, Place* place);
  void density_transmission_model(int day, int condition_id, Place* place);

  bool attempt_transmission(double transmission_prob, Person* infector, Person* infectee, int condition_id, int day, Place* place);
};


#endif // _FRED_RESPIRATORY_TRANSMISSION_H
