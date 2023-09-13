/*
 * Author: Meydi Wahendra <meydiwahendra@gmail.com> Copyright (c) 2023
 * 
 * License: GPL
 * A simple modules to optimize the custom kernel of Whyred (Redmi Note 5 Pro)

 * Adapted dsboost code to this modules.
 * Author: Tyler Nijmeh <tylernij@gmail.com> Copyright (c) 2019
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef DEOPT_H
#define DEOPT_H

extern int deserteagle_opt;
#ifdef CONFIG_DYNAMIC_STUNE_BOOST
void do_sched_boost_rem(void);
void do_sched_boost(void);
#else
static inline void do_sched_boost_rem(void)
{
}
static inline void do_sched_boost(void)
{
}
#endif /* CONFIG_DYNAMIC_STUNE_BOOST */

#endif /* DEOPT_H */
