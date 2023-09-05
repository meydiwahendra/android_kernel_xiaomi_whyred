#ifndef DEOPT_H
#define DEOPT_H

#include <linux/types.h>
#include <linux/power_supply.h>

extern int deserteagle_opt;

int smblib_set_prop_input_current_settled(struct power_supply *power_supply,
                                          const union power_supply_propval *val);

int smblib_set_prop_input_current_limited(struct power_supply *power_supply,
                                           const union power_supply_propval *val);

int smblib_set_prop_restricted_charging(struct power_supply *power_supply,
                                        const union power_supply_propval *val);

int smblib_set_prop_cool_temp(struct power_supply *power_supply,
                               const union power_supply_propval *val);

int smblib_set_prop_warm_temp(struct power_supply *power_supply,
                               const union power_supply_propval *val);

int smblib_set_prop_hot_temp(struct power_supply *power_supply,
                              const union power_supply_propval *val);

int smblib_set_prop_pd_allowed(struct power_supply *power_supply,
                                const union power_supply_propval *val);

int smblib_set_prop_allow_hvdcp3(struct power_supply *power_supply,
                                  const union power_supply_propval *val);

int smblib_set_prop_system_temp_level(struct power_supply *power_supply,
                                      const union power_supply_propval *val);

#endif /* DEOPT_H */
