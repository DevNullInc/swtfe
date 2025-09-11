/*****************************************************************************************
 *                       DDDDD        A        RRRRRRR     K    K                        *
 *                       D    D      A A       R      R    K   K                         *
 *                       D     D    A   A      R      R    KK K                          *
 *                       D     D   A     A     RRRRRRR     K K                           *
 *                       D     D  AAAAAAAAA    R    R      K  K                          *
 *                       D    D  A         A   R     R     K   K                         *
 *                       DDDDD  A           A  R      R    K    K                        *
 *                                                                                       *
 *                                                                                       *
 *W      WW      W    A        RRRRRRR   RRRRRRR   IIIIIIII    OOOO   RRRRRRR     SSSSS  *
 * W    W  W    W    A A       R      R  R      R     II      O    O  R      R   S       *
 * W    W  W    W   A   A      R      R  R      R     II     O      O R      R   S       *
 * W    W  W    W  A     A     RRRRRRR   RRRRRRR      II     O      O RRRRRRR     SSSSS  *
 *  W  W    W  W  AAAAAAAAA    R    R    R    R       II     O      O R    R           S *
 *  W W     W W  A         A   R     R   R     R      II      O    O  R     R          S *
 *   W       W  A           A  R      R  R      R  IIIIIIII    OOOO   R      R    SSSSS  *
 *                                                                                       *
 *****************************************************************************************
 *                                                                                       *
 * Dark Warrior Code additions and changes from the Star Wars Reality code copyright (c) *
 * 2003 by Michael Ervin, Mark Gottselig, Gavin Mogan                                    *
 *                                                                                       *
 * Star Wars Reality Code Additions and changes from the Smaug Code copyright (c) 1997   *
 * by Sean Cooper                                                                        *
 *                                                                                       *
 * Starwars and Starwars Names copyright(c) Lucas Film Ltd.                              *
 *****************************************************************************************
 *                                                                                       *
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson), Levi Beckerson (Whir),           *
 * Michael Ward (Tarl), Erik Wolfe (Dwip), Cameron Carroll (Cam), Cyberfox, Karangi,     *
 * Rathian, Raine, and Adjani. All Rights Reserved.                                      *
 *                                                                                       *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                                 Pfile Pruning Module                                  *
 ****************************************************************************************/

#include <string.h>
#include "mud.hpp"
#include "astral.hpp"

BodyData *get_body_here(SpaceData * star, char *name)
{
        BodyData *body = NULL;

        if (!star)
                return NULL;

        ForEachList(BodyList, star->bodies, body)
                if (!str_cmp(name, body->name()))
                return body;

        ForEachList(BodyList, star->bodies, body)
                if (nifty_is_name_prefix(name, const_cast<char*>(body->name())))
                return body;
        return NULL;
}

CMDF do_bomb(CharData * ch, char *argument)
{
        char      buf[MaxStringLength];
        BodyData *body = NULL;
        ShipData *ship = NULL;
        DescriptorData *d = NULL;
        int       chance, number, number2;

        if (argument[0] == '\0')
        {
                send_to_char("Syntax: bomb <target>\n\r", ch);
                return;
        }

        if (IsNpc(ch) || !ch->PCData || !ch->in_room)
        {
                send_to_char("What would be the point of that.\n\r", ch);
                return;
        }

        if (!(ship = ship_from_cockpit(ch->in_room->vnum)))
        {
                set_char_color(AtRed, ch);
                send_to_char
                        ("You have to be in the cockpit of a ship to bomb a planet",
                         ch);
                return;
        }

        act(AtPlain, "$n presses the fire button marked \"Space Bombs\".",
            ch, NULL, argument, ToRoom);
        act(AtPlain, "You press the fire button marked \"Space Bombs\".", ch,
            NULL, argument, ToChar);

        if (IsSet(ship->flags, ShipCloak))
        {
                echo_to_cockpit(AtRed, ship,
                                "&B[&zShip Weapons&B]&w You are unable todo so when cloaked.");
                return;
        }

        if ((body = get_body_here(ship->starsystem, argument)) == NULL)
        {
                send_to_char("You can't find that.", ch);
                act(AtPlain, "$n can't seem to find the target.", ch, NULL,
                    argument, ToRoom);
                return;
        }

        if (ch->position <= PosSleeping)
        {
                send_to_char("In your dreams or what?\n\r", ch);
                return;
        }

        if (ship->bombs <= 0)
        {
                echo_to_cockpit(AtRed, ship,
                                "&B[}RWARNING&B]&w You are out of bombs, trying to fire can damage your launcher!.");
                return;
        }

        if ((body->xpos() > ship->vx + 200) || (body->xpos() < ship->vx - 200)
            || (body->ypos() > ship->vy + 200)
            || (body->ypos() < ship->vy - 200)
            || (body->zpos() > ship->vz + 200)
            || (body->zpos() < ship->vz - 200))
        {
                send_to_char("&B[}RERROR&B] &RTarget is too far away!\n\r",
                             ch);
                return;
        }

        ship->bombs--;

        WaitState(ch, skill_table[gsn_bomb]->beats);

        chance = IsNpc(ch) ? ch->top_level : (int) (ch->PCData->
                                                     learned[gsn_bomb]);

        if (number_percent() > chance)
        {
                snprintf(buf, MSL,
                         "%s's space bomb veers off course and crashes into an uninhabited area of %s",
                         ship->name, body->name());
                echo_to_system(AtSay, ship, buf, NULL);
                echo_to_cockpit(AtYellow, ship,
                                "Your space bomb flies wide of the target.");
                return;
        }

        snprintf(buf, MSL,
                 "&B[}RNOTICE&B]&w Sensors indicate %s drops a space bomb onto %s.",
                 (IsSet(ship->flags, ShipStealth) ? "a ship" : ship->name),
                 body->name());
        echo_to_system(AtLblue, ship, buf, ship);
        snprintf(buf, MSL,
                 "&B[}RNOTICE&B]&w A space bomb drops slowly towards the surface of %s",
                 body->name());
        echo_to_cockpit(AtYellow, ship, buf);

        if (body->planet())
                body->planet()->pop_support =
                        URANGE(-100,
                               body->planet()->pop_support -
                               (ch->top_level / 50), 100);


        if (body->planet() && !IsSet(body->planet()->flags, PlanetShield))
        {
                CharData *vch;

                number = number_range(0, 4);
                number2 = number_range(0, 2);
/*		if ( number_percent() <= 20 && planet->controls > 0)
		{
			ch_printf(ch,"You notice that the shield flickers as you hit a shield generator.\n\r");
			planet->controls--;
			if ( planet->controls <= 0 )
			{
				RemoveBit(planet->flags, PlanetPshield);
				snprintf( buf, MSL, "&G&W[&R^zNOTICE&W^x] Sensors Indicate that %s's shields are now down" , planet->name);
				echo_to_system( AtSay , ship , buf , NULL );
				snprintf( buf, MSL, "&G&W[&R^zNOTICE&W^x] Sensors Indicate that %s's shields are now down" , planet->name);
				echo_to_cockpit( AtYellow , ship , buf );
			}
		}*/

                if (number_percent() <= 20
                    && body->planet()->defbattalions > 0)
                {
                        int       number = number_range(1, 5);

                        body->planet()->defbattalions =
                                UMAX(0,
                                     body->planet()->defbattalions - number);
                        send_to_char
                                ("The planetary barracks have been hit killing many soldiers.\n\r",
                                 ch);
                }
                for (d = first_descriptor; d; d = d->next)
                {
                        vch = d->character;
                        if (!IsPlaying(d))
                                continue;
                        if (!vch->in_room->area->planet)
                                continue;
                        if (!vch->in_room->area->planet->body &&
                            vch->in_room->area->planet->body != body)
                                continue;

                        if (IsOutside(vch))
                        {
                                switch (number)
                                {
                                case 0:
                                case 1:
                                case 2:
                                        send_to_char
                                                ("An explosion from a bomb shakes the ground violently!\n\r"
                                                 "And you loose your ballence and hit the ground hard!\n\r"
                                                 "&ROuch!\n\r\n\r", vch);
                                        vch->hit -= 100;
                                        break;
                                case 3:
                                case 4:
                                        send_to_char
                                                ("You hear screams nearby from a space bomb impact!\n\r"
                                                 "Luckly you are out of the blast radious\n\r",
                                                 vch);
                                        break;
                                default:
                                        send_to_char
                                                ("You hear screams nearby from a space bomb impact!\n\r"
                                                 "Luckly you are out of the blast radious\n\r",
                                                 vch);
                                        break;
                                }
                        }
                        else
                        {
                                switch (number2)
                                {
                                case 0:
                                        send_to_char
                                                ("An explosion from a bomb shakes the walls and the ground!\n\r"
                                                 "And you loose your ballence and hit the ground hard!\n\r"
                                                 "&ROuch!\n\r\n\r", vch);
                                        vch->hit -= 100;
                                        break;
                                case 1:
                                        send_to_char
                                                ("An explosion from a bomb causes small bits of ceiling tile to fall on your head.\n\r"
                                                 "&ROuch!\n\r\n\r", vch);
                                        vch->hit -= 500;
                                        break;
                                default:
                                        send_to_char
                                                ("You hear a bomb explode nearby, but this building seems unaffected!\n\r",
                                                 vch);
                                        break;
                                }
                        }
                        update_pos(vch);
                }
                echo_to_cockpit(AtYellow, ship,
                                "You notice the bomb exploding on the shielding.");
        }
        /*
         * End If pshield on 
         */
        else
        {
                CharData *vch;

/*		if ( number_percent() <= 50 && planet->controls > 0)
		{
			ch_printf(ch,"You notice that the shield flickers as you hit a shield generator.\n\r");
			planet->controls--;
			if ( planet->controls <= 0 )
			{
				RemoveBit(planet->flags, PlanetPshield);
				snprintf( buf, MSL, "&G&W[&R^zNOTICE&W^x] Sensors Indicate that %s's shields are now down" , planet->name);
				echo_to_system( AtSay , ship , buf , NULL );
				snprintf( buf, MSL, "&G&W[&R^zNOTICE&W^x] Sensors Indicate that %s's shields are now down" , planet->name);
				echo_to_cockpit( AtYellow , ship , buf );
			}
		}*/

                for (d = first_descriptor; d; d = d->next)
                {
                        vch = d->character;
                        if (!IsPlaying(d))
                                continue;
                        if (!vch->in_room->area->planet)
                                continue;
                        if (!vch->in_room->area->planet->body &&
                            vch->in_room->area->planet->body != body)
                                continue;

                        if (IsOutside(vch))
                                send_to_char
                                        ("Up above, you notice a bright light!",
                                         vch);
                        else
                                send_to_char
                                        ("You hear an explosion high above you.!",
                                         vch);
                }
                echo_to_cockpit(AtYellow, ship,
                                "You notice the bomb exploding on the surface");
        }

        gain_exp(ch, (10 /*Shield Mod */ ) * 100, PilotingAbility);
        ch_printf(ch, "You gain %d piloting experience.\n\r",
                  (10 /*shield Mod */ ) * 100);
        learn_from_success(ch, gsn_bomb);
        return;
}

CMDF do_dock(CharData * ch, char *argument)
{
        int       percent_chance;
        ShipData *ship;
        ShipData *target;
        char      buf[MaxStringLength];

        if (argument[0] == '\0')
        {
                send_to_char("&RDock to what?\n\r", ch);
                return;
        }


        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }


        if (!check_pilot(ch, ship))
        {
                send_to_char("This isn't your ship!\n\r", ch);
                return;
        }

        if (ship->pilotseat != ch->in_room->vnum)
        {
                send_to_char("&RYou need to be in the pilot seat!\n\r", ch);
                return;
        }


        if (ship->shipstate == ShipDisabled)
        {
                send_to_char("&RYour ship is disabled. You can't dock.\n\r",
                             ch);
                return;
        }

        if (ship->shipstate == ShipHyperspace)
        {

                send_to_char("&RYou can only do that in realspace!\n\r", ch);
                return;
        }

        if (ship->shipstate != ShipReady)
        {
                send_to_char
                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                         ch);
                return;
        }

        target = get_ship_here(argument, ship->starsystem);

        if (target == NULL)
        {
                send_to_char("&RI don't see that here.\n\r", ch);
                return;
        }

        if (target == ship)
        {
                send_to_char("&RYou can't dock yourself!\n\r", ch);
                return;
        }

        if (target->shipstate != ShipDisabled && target->currspeed > 0)
        {
                send_to_char
                        ("&RThat ship is not suffeciently disabled to dock with it!\n\r",
                         ch);
                return;
        }

        if (target->shipstate == ShipLand)
        {
                send_to_char
                        ("&RThat ship is already in a landing sequence.\n\r",
                         ch);
                return;
        }

        if ((target->vx > ship->vx + (ship->currspeed + 50))
            || (target->vx < ship->vx - (ship->currspeed + 50))
            || (target->vy > ship->vy + (ship->currspeed + 50))
            || (target->vy < ship->vy - (ship->currspeed + 50))
            || (target->vz > ship->vz + (ship->currspeed + 50))
            || (target->vz < ship->vz - (ship->currspeed + 50)))
        {
                send_to_char
                        ("&R That ship is too far away! You'll have to fly a litlle closer.\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > target->ship_class)
        {
                send_to_char("&RYour ship is too big to dock.\n\r", ch);
                return;
        }

        if (target->ship_class == ShipPlatform)
        {
                send_to_char("&RYou can't dock to a platforms.\n\r", ch);
                return;
        }

        if (ship->energy < (25 + 25 * target->ship_class))
        {
                send_to_char("&RTheres not enough fuel!\n\r", ch);
                return;
        }

        percent_chance =
                IsNpc(ch) ? ch->top_level : (int) (ch->PCData->
                                                    learned[gsn_dock]);

        /*
         * This is just a first guess percent_chance modifier, feel free to change if needed 
         */

        if (number_percent() < percent_chance)
        {
                set_char_color(AtGreen, ch);
                send_to_char("Docking sequence initiated.\n\r", ch);
                act(AtPlain, "$n begins to dock with the target.", ch, NULL,
                    argument, ToRoom);
                echo_to_ship(AtYellow, target,
                             "ALERT: Your ship is being docked!");
                snprintf(buf, MSL, "You are being docked by %s.", ship->name);
                echo_to_cockpit(AtBlood, target, buf);
                WaitState(ch, 10);
                echo_to_ship(AtRed, ship, "Docking sequence Complete.\n\r");

                ship->currspeed = target->currspeed = 0;
                ship->vx = target->vx;
                ship->vy = target->vy;
                ship->vz = target->vz;
                ship->hy = target->hy;
                ship->hz = target->hz;
                ship->hx = target->hx;
                ship->dockedto = target;
                target->dockedto = ship;
                target->target0 = ship->target0 = NULL;
                target->target1 = ship->target1 = NULL;
                target->target2 = ship->target2 = NULL;

                learn_from_success(ch, gsn_dock);
                return;

        }

        damage_ship(ship, 10, 20);
        damage_ship(target, 10, 20);
        send_to_char("You fail to work the controls properly.\n\r", ch);
        echo_to_ship(AtYellow, target,
                     "The ship shudders and then continues as someone tries to dock to you!.");
        echo_to_ship(AtYellow, ship,
                     "The ship shudders and then continues as the ship tries to dock with someone!.");
        snprintf(buf, MSL, "The %s attempted to dock your ship!", ship->name);
        echo_to_cockpit(AtBlood, target, buf);
        if (autofly(target) && !target->target0)
                target->target0 = ship;

        learn_from_failure(ch, gsn_dock);

        return;
}

CMDF do_undock(CharData * ch, char *argument)
{
        int       percent_chance;
        ShipData *ship;
        char      buf[MaxStringLength];

        argument = NULL;

        percent_chance =
                IsNpc(ch) ? ch->top_level : (int) (ch->PCData->
                                                    learned[gsn_dock]);
        if (percent_chance == 0)
        {
                /*
                 * interp("dock", ch) or UnknownCommand instead of magic phrase - Gavin 
                 */
                send_to_char("Huh?", ch);
                return;
        }


        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if (ship->dockedto == NULL)
        {
                send_to_char("You arn't docked.\n\r", ch);
                return;
        }

        if (!check_pilot(ch, ship))
        {
                send_to_char("This isn't your ship!\n\r", ch);
                return;
        }

        if (ship->pilotseat != ch->in_room->vnum)
        {
                send_to_char("&RYou need to be in the pilot seat!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipDisabled)
        {
                send_to_char("&RYour ship is disabled. You can't undock.\n\r",
                             ch);
                return;
        }

        if (ship->shipstate != ShipReady)
        {
                send_to_char
                        ("&RPlease wait until the ship has finished its current manouver.\n\r",
                         ch);
                return;
        }

        if (ship->energy < (25 + 25 * ship->dockedto->ship_class))
        {
                send_to_char("&RTheres not enough fuel!\n\r", ch);
                return;
        }

        percent_chance =
                IsNpc(ch) ? ch->top_level : (int) (ch->PCData->
                                                    learned[gsn_dock]);

        /*
         * This is just a first guess percent_chance modifier, feel free to change if needed 
         */

        if (number_percent() < percent_chance)
        {
                set_char_color(AtGreen, ch);
                send_to_char("Undocking sequence initiated.\n\r", ch);
                act(AtPlain, "$n begins detaching with $t.", ch,
                    ship->dockedto->name, NULL, ToRoom);
                echo_to_ship(AtYellow, ship->dockedto,
                             "ALERT: Your ship is undocking!");
                snprintf(buf, MSL, "You are seperating from %s.", ship->name);
                echo_to_cockpit(AtBlood, ship->dockedto, buf);
                WaitState(ch, 10);
                echo_to_ship(AtRed, ship, "Ship seperation Complete.\n\r");
                echo_to_ship(AtRed, ship->dockedto,
                             "Ship seperation Complete.\n\r");

                if (ship->dockedto->hatchopen)
                {
                        echo_to_ship(AtRed, ship->dockedto,
                                     "Warning: Hatch left open, damaging shipe.\n\r");
                        damage_ship(ship->dockedto, 10, 20);
                        ship->dockedto->hatchopen = FALSE;
                }
                if (ship->hatchopen)
                {
                        echo_to_ship(AtRed, ship,
                                     "Warning: Hatch left open, damaging shipe.\n\r");
                        damage_ship(ship, 10, 20);
                        ship->hatchopen = FALSE;
                }
                ship->dockedto->dockedto = NULL;
                ship->dockedto = NULL;
                learn_from_success(ch, gsn_dock);
                return;

        }

        damage_ship(ship, 10, 20);
        damage_ship(ship->dockedto, 10, 20);
        send_to_char("You fail to work the controls properly.\n\r", ch);
        echo_to_ship(AtYellow, ship->dockedto,
                     "The ship shudders and sounds of scraping can be heard as the ships try to seperate.");
        echo_to_ship(AtYellow, ship,
                     "The ship shudders and sounds of scraping can be heard as the ships try to seperate.");

        learn_from_failure(ch, gsn_dock);

        return;
}

CMDF do_extrapolate(CharData * ch, char *argument)
{
        BodyData *body;
        int       percent_chance, xp;
        ShipData *ship;

        if (argument[0] == '\0')
        {
                send_to_char("&RWhat body do you wish to extrapolate!\n\r",
                             ch);
                return;
        }
        if ((ship = ship_from_cockpit(ch->in_room->vnum)) == NULL)
        {
                send_to_char
                        ("&RYou must be in the cockpit of a ship to do that!\n\r",
                         ch);
                return;
        }

        if (ship->ship_class > ShipPlatform)
        {
                send_to_char("&RThis isn't a spacecraft!\n\r", ch);
                return;
        }

        if (ship->shipstate == ShipDocked)
        {
                send_to_char
                        ("&RYou can't do that until after you've launched!\n\r",
                         ch);
                return;
        }

        if (ship->navseat != ch->in_room->vnum)
        {
                send_to_char
                        ("&RYou must be at a nav computer to extrapolate orbits.\n\r",
                         ch);
                return;
        }

        percent_chance =
                IsNpc(ch) ? ch->top_level : ch->PCData->
                learned[gsn_extrapolate];
        if (percent_chance < number_percent())
        {
                learn_from_failure(ch, gsn_juke);
                send_to_char("&RYou can't make any sense of the charts!\n\r",
                             ch);
                return;
        }

        if ((body = get_body(argument)) == NULL)
        {
                send_to_char("&RYou can't find that.\n\r", ch);
                return;
        }

        send_to_char("&YYou calculate the planets current coordinates\n\r",
                     ch);

        send_to_char
                ("&C---------------------------------------------------------------\n\r",
                 ch);
        set_char_color(AtLblue, ch);
        ch_printf(ch, " %-20.20s %-5d %-5d %-5d (%s)\n\r",
                  body->name(), body->xpos(), body->ypos(), body->zpos(),
                  percent_chance > 50
                  && body->starsystem()? body->starsystem()->
                  name : "Unknown");
        send_to_char
                ("&C---------------------------------------------------------------\n\r",
                 ch);

        xp = (number_percent());
        gain_exp(ch, xp, PilotingAbility);
        ch_printf(ch, "&YYou gain %ld piloting experience!\n\r", xp);
        learn_from_success(ch, gsn_extrapolate);
        return;
}
