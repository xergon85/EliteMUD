/* ************************************************************************
*   File: functions.h                                   Part of EliteMUD  *
*  Usage: Header file with all function proto types                       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1994 Mr Wang at RIT Stockholm Sweden                     *
*  EliteMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
************************************************************************ */


/* This headerfile SHOULD contain all the function prototypes.
 * Whenever you write a new function, write the prototype here in the correct
 * place.
 * This gives a certain safety and comapbility test during compilation.
 */



/* FROM: act.comm.c
 */

void add_racial_sentence(struct char_data *ch, char *sent);
void perform_tell(struct char_data *ch, struct char_data *vict, char *arg);
void info_line(struct char_data *ch, struct char_data *victim, char *message);


/* FROM: act.informative.c
 */

struct char_data * obj_carried_by(struct obj_data *obj);
void argument_split_2(char *argument, char *first_arg, char *second_arg);
void display_string_array(struct char_data *ch, char **array);
char *find_ex_description(char *word, struct extra_descr_data *list);
void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode);
void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, bool show);
void	diag_char_to_char(struct char_data *i, struct char_data *ch);
char *diag_to_prompt(struct char_data *i, int mode);
void blind_routine(struct char_data *ch);
void	show_char_to_char(struct char_data *i, struct char_data *ch, int mode);
void	list_char_to_char(struct char_data *list, struct char_data *ch, int mode);
void perform_mortal_where(struct char_data *ch, char *arg);
void perform_immort_where(struct char_data *ch, char *arg);
int combat_rating(struct char_data *ch);
void	sort_commands(void);
int  write_to_pos(struct descriptor_data *d, int x, int y, char *text, int mode);
char * convert_to_color(struct descriptor_data *d, char *text);
void  diag_to_screen(struct descriptor_data *d);
void  stats_to_screen(struct descriptor_data *d);
void  aff_to_screen(struct descriptor_data *d);
void  line_to_screen(struct descriptor_data *d);
void  redraw_screen(struct descriptor_data *d);


/* FROM: act.movement.c
 */

char * leave_enter_string(struct char_data *ch);
int	do_simple_move(struct char_data *ch, int cmd, int following);
int chaotic_move(struct char_data *ch,int direction);
int	find_door(struct char_data *ch, char *type, char *dir);
int	has_key(struct char_data *ch, int key);



/* FROM: act.obj1.c
 */

int perform_object_handling(struct char_data *ch, struct char_data *vict, struct obj_data *ls, struct obj_data *container, char *sname, int mode, sh_int RDR, int amount);
int highest_obj_level(struct obj_data *obj);
int	can_take_obj(struct char_data *ch, struct obj_data *obj);
int	get_check_money(struct char_data *ch, struct obj_data *obj);
void	get_from_container(struct char_data *ch, struct obj_data *cont, char *arg, int mode, int amount);
void get_from_room(struct char_data *ch, char *arg, int amount);
void	perform_drop_gold(struct char_data *ch, int amount, byte mode, sh_int RDR);
struct char_data *give_find_vict(struct char_data *ch, char *arg1);
void	perform_give_gold(struct char_data *ch, struct char_data *vict, int amount);


/* FROM: act.obj2.c
 */

int     damage_object(struct obj_data *obj, int dam);
void	weight_change_object(struct obj_data *obj);
void	name_from_drinkcon(struct obj_data *obj);
void	name_to_drinkcon(struct obj_data *obj, int type);
void	wear_message(struct char_data *ch, struct obj_data *obj, int where);
void	perform_wear(struct char_data *ch, struct obj_data *obj, int where);
int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);
void	perform_remove(struct char_data *ch, int pos);


/* FROM: act.offensive.c
 */

int pkok_check(struct char_data *ch, struct char_data *victim);

/* FROM: act.other.c
 */

void  improve_skill(struct char_data *ch, int skillnr);
void  chlist_to_char(struct char_data *list, struct char_data *ch, int range);
void show_clan_info(struct char_data *ch, int vnum, int info_check);
void afs_force_save(void);

/* FROM: act.social.c
 */

char	*fread_action(FILE *fl, int nr);
void	boot_social_messages(void);
int	find_action(int cmd);
void send_to_clan(struct char_data *ch, char *arg);
void clan_update(void);


/* FROM: act.wizard.c
 */

sh_int find_target_room(struct char_data *ch, char *rawroomstr);
void	do_stat_room(struct char_data *ch, struct room_data *rm);
void do_stat_object(struct char_data *ch, struct obj_data *j);
char * typename(int nr);
void do_stat_character(struct char_data *ch, struct char_data *k);
void	roll_abilities(struct char_data *ch);
void	do_start(struct char_data *ch);
void	print_zone_to_buf(char *bufptr, int zone);
int immortal_interpreter(struct char_data *ch, char *command);
void race_config(struct char_data *vict);


/* FROM: ban.c
 */

void	load_banned (void);
int	isbanned(char *hostname);
/* void	_write_one_node(FILE *fp, struct ban_list_element *node); */
void	write_ban_list(void);
int	Valid_Name(char *newname);
void	Read_Invalid_List(void);


/* FROM: boards.c
 */

int	find_slot(void);
int	find_board(struct obj_data *obj);
void	init_boards(void);
int	Board_display_msg(int board_type, struct char_data *ch, struct obj_data *obj, char *arg);
int	Board_show_board(int board_type, struct char_data *ch, struct obj_data *ovj, char *arg);
int	Board_remove_msg(int board_type, struct char_data *ch, char *arg);
void	Board_save_board(int board_type);
void	Board_load_board(int board_type);
void	Board_reset_board(int board_num);
void	Board_write_message(int board_type, struct char_data *ch, char *arg);
int     Board_edit_msg(int board_type, struct char_data *ch, char *arg);
int     Board_lock_msg(int board_type, struct char_data *ch, char *arg);
int     Board_unlock_msg(int board_type, struct char_data *ch, char *arg);
int     msg_locked(int board_type, int ind);
int     lock_board(int board_type, struct char_data *ch);
int     unlock_board(int board_type, struct char_data *ch);
int     unlock_msg(int board_type, int ind, struct char_data *ch);


/* FROM: castle.c
 */

void	assign_kings_castle (void);
int	member_of_staff (struct char_data *chChar);
int	member_of_royal_guard (struct char_data *chChar);
struct char_data *find_npc_by_name (struct char_data *chAtChar, char *pszName,
int iLen);
struct char_data *find_guard (struct char_data *chAtChar);
struct char_data *get_victim (struct char_data *chAtChar);
int	banzaii (struct char_data *ch);
int	do_npc_rescue (struct char_data *ch_hero, struct char_data *ch_victim);
int	block_way(struct char_data *ch, int cmd, char *arg, int iIn_room, int iProhibited_direction);
int	is_trash (struct obj_data *i);
void	fry_victim (struct char_data *ch);

/* FROM: comm.c
 */

int get_avail_descs(void);
void	run_the_game(int port);
void	game_loop(int s);
int	get_from_q(struct txt_q *queue, char *dest);
void	write_to_output(const char *txt, struct descriptor_data *t);
void	write_to_q(char *txt, struct txt_q *queue);
struct timeval timediff(struct timeval *a, struct timeval *b);
void	flush_queues(struct descriptor_data *d);
int init_socket(int port);
void hostname(struct descriptor_data *desc, int slown);
int	new_connection(int s);
int	new_descriptor(int s);
void  str_color_cat(struct descriptor_data *d, char *to, char *from);
int	process_output(struct descriptor_data *t);
int	write_to_descriptor(socket_t desc, char *txt);
int	process_input(struct descriptor_data *t);
int	perform_subst(struct descriptor_data *t, char *orig, char *subst);
void	close_sockets(int s);
void	close_socket(struct descriptor_data *d);
void	nonblock(socket_t s);
void	send_to_char(char *messg, struct char_data *ch);
void	send_to_all(char *messg);
void	send_to_outdoor(char *messg);
void	send_to_except(char *messg, struct char_data *ch);
void	send_to_room(char *messg, int room);
void	send_to_room_except(char *messg, int room, struct char_data *ch);
void	send_to_room_except_two(char *messg, int room, struct char_data *ch1, struct char_data *ch2);
void	act(char *str, int hide_invisible, struct char_data *ch, struct obj_data *obj, void *vict_obj, int type);
void   check_gain(void);
int arss_counter(char *str);
char *arss_forward(char *str, int num);
char *arss_findend(char *str);
char *arss_randompointer(char *ptr);

/* FROM: config.c
 */

/* FROM: constants.c
 */


/* FROM: db.c
 */

void    boot_clanpower(void);
void	reboot_wizlists(void);
void	boot_db(void);
void	reset_time(void);
void	build_player_index(void);
int	count_hash_records(FILE *fl);
void	index_boot(int mode);
void    boot_questeq();
char * fread_share_string(FILE *fl, char *errorbuf, int num, int type);
void    setup_crashrooms();
void    arrange_zones();
void	load_rooms(FILE *fl);
void	setup_dir(FILE *fl,struct room_data *room, int nr, int dir);
void	renum_world(void);
void	renum_zone_table(void);
void	load_zones(FILE *fl);
void	load_clans(FILE *clan_f);
void	check_start_rooms(void);
struct char_data *read_mobile(int nr, int type);
void	load_mobiles(FILE *mob_f);
struct obj_data *read_object(int nr, int type);
void	load_objects(FILE *obj_f);
void	zone_update(void);
void	reset_zone(int zone, bool complete);
/* void    check_reset_cmds(struct reset_com *ptr, char *errorbuf); */
int	is_empty(int zone_nr);
int	load_char(char *name, struct char_file_u *char_element);
void	store_to_char(struct char_file_u *st, struct char_data *ch);
void	char_to_store(struct char_data *ch, struct char_file_u *st, bool u_time, bool u_logon);
int	create_entry(char *name);
void	save_char(struct char_data *ch, sh_int load_room, int mode); 
char    fread_letter(FILE *fp);
char	*fread_string(FILE *fl, char *error);
void fwrite_string(FILE *fl, char *str);
long  fread_number(FILE *fl, char *error);
void free_alias_list(struct alias_list *ls);
void	free_char(struct char_data *ch);
void	free_obj(struct obj_data *obj);
int	file_to_string_alloc(char *name, char **buf);
int	file_to_string(char *name, char *buf);
void	reset_char(struct char_data *ch);
void	clear_char(struct char_data *ch);
void	clear_object(struct obj_data *obj);
void	init_char(struct char_data *ch);
int	real_room(int vnum);
int	real_mobile(int virtual);
int	real_object(int virtual);
int     real_zone(int virtual);
int     real_clan(int virtual);
void set_key_timer(struct obj_data *obj);



/* FROM: fight.c
 */

void	appear(struct char_data *ch);
void    fall_of_mount(struct char_data *ch);
void	load_messages(void);
void	update_pos( struct char_data *victim );
void	check_killer(struct char_data *ch, struct char_data *vict);
void	set_fighting(struct char_data *ch, struct char_data *vict);
void    stop_fighting(struct char_data *ch);
void	make_corpse(struct char_data *ch);
void    change_alignment(struct char_data * ch, struct char_data * victim);
void	death_cry(struct char_data *ch);
void    raw_kill(struct char_data * ch, struct char_data * killer);
void    die(struct char_data * ch, struct char_data * killer);
void	group_gain(struct char_data *ch, struct char_data *victim);
char	*replace_string(char *str, char *weapon_singular, char *weapon_plural);
void	dam_message(int dam, struct char_data *ch, struct char_data *victim, int w_type);
void	damage(struct char_data *ch, struct char_data *victim, int dam, int attacktype);
int get_thaco(struct char_data *ch);
void	hit(struct char_data *ch, struct char_data *victim, int type);
void	perform_violence(void);
int group_xp_value(struct char_data *ch);



/* FROM: gen_cards.c
 */

void   free_card_list(struct card_list *ls);
void   init_card_deck(struct char_data *ch);
int   count_deck(struct card_list *ls);
void   shuffle_deck(struct char_data *ch);
void   display_cards(struct char_data *ch, int show);
void   deal_cards_to_char(struct char_data *ch, struct char_data *victim, int num);
void  return_all_cards(struct char_data *ch, struct card_list *ls);
void  return_all_in_hand(struct char_data *ch, struct char_data *victim);
void  return_played_cards(struct char_data *ch);
struct card_list *  drop_cards(struct char_data *ch, char *arg);
int   switch_cards(struct char_data *ch, int first, int second);
void  sort_cards(struct  card_list *ls);
void sort_cards_in_hand(struct char_data *ch);


/* FROM: graph.c
 */

void bfs_enqueue(sh_int room, char dir, ubyte steps);
void bfs_dequeue(void);
void bfs_clear_queue(void);
void stack_push(struct track_stack_data **stk, byte dir, sh_int room);
int stack_pop(struct track_stack_data **stk);
void free_stack(struct track_stack_data *stk);
struct track_stack_data * build_track_stack(sh_int src, sh_int target);
struct track_stack_data * make_track_stack(sh_int src, sh_int target, ubyte maxsteps);
void track_check(struct char_data *ch);
void hunt_victim(struct char_data *ch);
int perform_track(struct char_data *ch, sh_int target, sbyte steps);


/* FROM: handler.c
 */

char	*fname(char *namelist);
int	isname(char *str, char *namelist);
void    stat_check(struct char_data *ch);
void    innate_check(struct char_data *ch);
void	affect_modify(struct char_data *ch, byte loc, sbyte mod, long bitv, bool add);
void	affect_total(struct char_data *ch);
void	affect_to_char( struct char_data *ch, struct affected_type *af);
void	affect_remove( struct char_data *ch, struct affected_type *af );
void	affect_from_char( struct char_data *ch, int skill);
bool affected_by_spell( struct char_data *ch, int skill );
void	affect_join( struct char_data *ch, struct affected_type *af, bool avg_dur, bool avg_mod);
void	affect_join( struct char_data *ch, struct affected_type *af, bool avg_dur, bool avg_mod);
void	affect_to_room(sh_int roomnr, struct room_affect_type *af);
void	affect_from_room(sh_int roomnr, int spell);
void insert_to_char_list(struct char_data *ch);
void extract_from_char_list(struct char_data *ch);
void mob_to_character_list(struct char_data *mob, struct char_data **ptr);
void  bubble_char_to_list(struct char_data *ch, struct char_data **ptr);
void	char_to_room(struct char_data *ch, int room);
void    char_from_room(struct char_data *ch);
void  bubble_obj_to_list(struct obj_data **obj, struct obj_data **ptr);
void	obj_to_char(struct obj_data *object, struct char_data *ch);
void	obj_from_char(struct obj_data *object);
int	apply_ac(struct char_data *ch, int eq_pos);
void	equip_char(struct char_data *ch, struct obj_data *obj, int pos);
struct obj_data *unequip_char(struct char_data *ch, int pos);
int	get_number(char **name);
struct obj_data *get_obj_in_list(char *name, struct obj_data *list);
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);
struct obj_data *get_obj(char *name);
struct obj_data *get_obj_num(int nr);
struct char_data *get_char_room(char *name, int room);
struct char_data *get_char(char *name);
struct char_data *get_char_num(int nr);
void	obj_to_room(struct obj_data *object, int room);
void	obj_from_room(struct obj_data *object);
void	obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void	obj_from_obj(struct obj_data *obj);
void	object_list_new_owner(struct obj_data *list, struct char_data *ch);
void	extract_obj(struct obj_data *obj);
void	update_object( struct obj_data *obj, int use);
void	update_char_objects( struct char_data *ch );
void	extract_char(struct char_data *ch);
struct char_data *get_char_room_vis(struct char_data *ch, char *name);
struct char_data *get_player_vis(struct char_data *ch, char *name);
struct char_data *get_player_vis_exact(struct char_data *ch, char *name);
struct char_data *get_char_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list);
struct obj_data *get_obj_vis(struct char_data *ch, char *name);
struct obj_data *get_object_in_equip_vis(struct char_data *ch, char *arg, struct obj_data *equipment[], int *j);
struct obj_data *create_money(int amount);
int	generic_find(char *arg, int bitvector, struct char_data *ch, struct char_data **tar_ch, struct obj_data **tar_obj);
int	find_all_dots(char *arg);
sh_int  find_room_by_name(char *roomname);
int     clan_rnum(int vnum);
int  validate_char(struct char_data *ch);


/* FROM: interpreter.c
 */

int	search_block(char *arg, char **list, bool exact);
void   process_speedwalk(struct char_data *ch, char *comline);
int    process_numeric_command(struct char_data *ch, char **comline);
void    process_multi_commands(struct char_data *ch, char *comline);
void    alias_interpreter(struct char_data *ch, char *comline);
void	command_interpreter(struct char_data *ch, char *argument);
int	is_number(char *str);
int	fill_word(char *argument);
char *one_argument(char *argument, char *first_arg);
char *any_one_arg(char *argument, char *first_arg);
char *two_arguments(char *argument, char *first_arg, char *second_arg);
int	is_abbrev(char *arg1, char *arg2);
void	half_chop(char *string, char *arg1, char *arg2);
int     find_command(char *command);
int	special(struct char_data *ch, int cmd, char *arg);
int mob_do_action(int type, struct char_data *ch, struct char_data *victim);
void	assign_command_pointers (void);
int	find_name(char *name);
int	_parse_name(char *arg, char *name);
void  count_worshippers(struct char_data *ch);
char * get_deity_name(struct char_data *ch);
void display_displays(struct descriptor_data *d);
void display_races(struct descriptor_data *d);
void display_classes(struct descriptor_data *d);
void	nanny(struct descriptor_data *d, char *arg);
char *delete_doubledollar(char *string);


/* FROM: limits.c
 */

void improve_abilities(struct char_data *ch, int class);
void set_lowest_level(struct char_data *ch, int newlevel);
int exp_needed(struct char_data *ch);
int	mana_gain(struct char_data *ch);
int	hit_gain(struct char_data *ch);
int	move_gain(struct char_data *ch);
void	advance_level(struct char_data *ch, int log);
void	set_title(struct char_data *ch);
void check_autowiz(struct char_data *ch);
void	gain_exp(struct char_data *ch, int gain);
void	gain_exp_regardless(struct char_data *ch, int gain);
void	gain_condition(struct char_data *ch, int condition, int value);
void	check_idling(struct char_data *ch);
void	point_update( void );


/* FROM: magic.c
 */

void mag_damage(int level, struct char_data * ch, struct char_data * victim,
		int spellnum, int casttype);
void mag_areas(byte level, struct char_data * ch, int spellnum, int savetype);
void mag_points(int level, struct char_data * ch, struct char_data * victim,
		int spellnum, int casttype);
void mag_affects(int level, struct char_data * ch, struct char_data * victim,
		 int spellnum, int casttype);
void mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
		   int spellnum, int type);
void mag_creations(int level, struct char_data * ch, int spellnum);
void mag_summons(int level, struct char_data * ch, struct obj_data * obj,
		 int spellnum, int savetype);
void mag_groups(int level, struct char_data * ch, int spellnum, int casttype);


/* FROM: mail .c
 */

void	push_free_list(long pos);
long	pop_free_list(void);
/* mail_index_type *find_char_in_index(char *searchee); */
void	write_to_file(void *buf, int size, long filepos);
void	read_from_file(void *buf, int size, long filepos);
void	index_mail(char *raw_name_to_index, long pos);
int	scan_file(void);
int	has_mail(char *recipient);
void	store_mail(char *to, char *from, char *message_pointer);
char	*read_delete(char *recipient, char *recipient_formatted);
struct char_data *find_mailman(struct char_data *ch);
void	postmaster_send_mail(struct char_data *ch, struct char_data *mailman, int cmd, char *arg);
void	postmaster_check_mail(struct char_data *ch, struct char_data *mailman, int cmd, char *arg);
void	postmaster_receive_mail(struct char_data *ch, struct char_data *mailman, int cmd, char *arg);
void postmaster_list_postcards(struct char_data * ch, struct char_data *mailman, int cmd, char *arg);
void postmaster_examine_postcard(struct char_data * ch, struct char_data *mailman, int cmd, char *arg);
char * get_mail_date(char *recepient);

/* FROM: mobact.c
 */

char * get_command_line(char *from, char *to);
void  get_next_mob_command(struct char_data *mob, char *cmd);
void   exec_mob_command(struct char_data *mob, char *cmd, long flags);
void	mobile_activity(void);
void	remember (struct char_data *ch, struct char_data *victim);
void	forget (struct char_data *ch, struct char_data *victim);
void	clearMemory(struct char_data *ch);
void annoy_hunted_victim(struct char_data *ch, struct char_data *vict, int chance);

/* FROM: modify.c
 */

void	string_add(struct descriptor_data *d, char *str);
void	quad_arg(char *arg, int *type, char *name, int *field, char *string);
char	*one_word(char *argument, char *first_arg );
struct help_index_element *build_help_index(FILE *fl, int *num);
void  return_ansi_cursor(struct descriptor_data *d);
void	page_string(struct descriptor_data *d, char *str, int keep_internal);
void	show_string(struct descriptor_data *d, char *input);
void	check_reboot(void);
void write_last_command(char *arg);



/* FROM: objsave.c
   */

int	get_filename(char *orig_name, char *filename, int mode);
int	Crash_delete_file(char *name, int mode);
int	Crash_delete_crashfile(struct char_data *ch);
int	Crash_clean_file(char *name);
void	update_obj_file(void) ;
struct obj_data *store_to_obj(struct obj_file_elem *object); 
void	Crash_obj2char(struct char_data *ch, struct obj_file_elem *object);
void	Crash_clear_weight(struct obj_data *obj);
void	Crash_restore_weight(struct obj_data *obj);
void	Crash_listrent(struct char_data *ch, char *name);
/* int	Crash_write_rentcode(struct char_data *ch, FILE *fl, struct rent_info *rent); */
int	Crash_load(struct char_data *ch);
int	storesave_obj(struct obj_data *obj, int pos, FILE *fl);
int	Crash_obj2store(struct obj_data *obj, struct char_data *ch, FILE *fl);
int save_obj(struct obj_data *obj, int pos, FILE *fl);
int   save_char_objs(struct char_data *ch, FILE *fl);
int	Crash_save(struct obj_data *obj, struct char_data *ch, FILE *fp);
void	Crash_extract_objs(struct obj_data *obj, int *num);
int	Crash_is_unrentable(struct obj_data *obj);
void	Crash_extract_norents(struct obj_data *obj);
int	Crash_is_junk(struct obj_data *obj);
void	Crash_extract_junk(struct obj_data *obj, struct char_data *ch);
void	Crash_extract_expensive(struct obj_data *obj);
void	Crash_calculate_rent(struct obj_data *obj, int *cost);
void	Crash_crashsave(struct char_data *ch, int mode);
void	Crash_idlesave(struct char_data *ch);
void	Crash_rentsave(struct char_data *ch, int cost);
void	Crash_cryosave(struct char_data *ch, int cost);
void	Crash_quitsave(struct char_data *ch);
int	Crash_report_unrentables(struct char_data *ch, struct char_data *recep,
struct obj_data *obj);
void	Crash_report_rent(struct char_data *ch, struct char_data *recep, 
struct obj_data *obj, long *cost, long *nitems, int display, int factor);
int	Crash_offer_rent(struct char_data *ch, struct char_data *receptionist,
int display, int factor);
int	gen_receptionist(struct char_data *ch, int cmd, char *arg, int mode);
int    Crash_load_rooms(void);
void	Crash_crashsave_room(sh_int roomnr);
void	Crash_save_all(void);
void  stringdata_save(struct char_data *ch);
void  alias_save(struct char_data *ch);
int strlen_w_o_blanks(char *s);
void  alias_load(struct char_data *ch);
void delete_alias_file(struct char_data *ch);
void  stringdata_load(struct char_data *ch);
void list_plog(struct char_data *ch, char *name);
char *write_plog(char *file, char *output);

/* FROM: shop.c
 */

int	is_ok(struct char_data *keeper, struct char_data *ch, int shop_nr);
int	trade_with(struct obj_data *item, int shop_nr);
int	shop_producing(struct obj_data *item, int shop_nr);
void	shopping_buy( char *arg, struct char_data *ch, struct char_data *keeper, int shop_nr);
void	assign_the_shopkeepers(void);
void    boot_the_shops(FILE *shop_f, char *filename);


/* FROM: spec_assign.c
 */

void	assign_mobiles(void);
void	assign_objects(void);
void	assign_rooms(void);
void special_assign(char* str, ubyte type, int  (**ptr)(struct char_data *ch, struct char_data *mob, struct obj_data *obj, int cmd, char *arg), char *errbuf);
char *
spec_proc_by_name(int  (*spec_pointer)(struct char_data *ch, struct char_data *mob, struct obj_data *obj, int cmd, char *arg));

/* FROM: spec_procs.c
 */

char	*how_good(int percent);
char	*how_hard(int value);
int know_skill(struct char_data *ch, int skillnr);
int get_skill_max(struct char_data *ch, int skillnr);
int get_skill_diff(struct char_data *ch, int skillnr);
void  practices(struct char_data *ch, int mode);
int guild(struct char_data *ch, struct char_data *mob, char *arg);
void	exec_social(struct char_data *npc, char *cmd, int next_line,
int *cur_line, void **thing);


/* FROM: spell_parser.c
 */

void	affect_update( void );
void	clone_obj(struct obj_data *obj);
bool elite_follow(struct char_data *ch, struct char_data *victim);
int allow_follower(struct char_data *ch);
void	stop_follower(struct char_data *ch);
void	die_follower(struct char_data *ch);
void  die_mount(struct char_data *ch);
void  die_protector(struct char_data *ch);
void	add_follower(struct char_data *ch, struct char_data *leader);
void	say_spell(struct char_data *ch, int si );
int saves_spell(struct char_data *ch, sh_int save_type, int *dam, int type);
void  skip_spaces(char **string);
int call_magic(struct char_data *ch, char *arg, struct char_data *victim,
	       struct obj_data *tar_obj, int spellnum,
	       int level, int casttype);
void mag_objectmagic(struct char_data * ch, struct obj_data * obj,
		     char *argument);
int mob_cast_spell(struct char_data *ch, 
		   struct char_data *victim,
		   int spellnum, 
		   sbyte percent,
		   byte flag,
		   int value1,
		   byte value2);
void  spello(int nr, int type, byte pos, ubyte mana, byte beat,
             sh_int targs, byte off, char* mess);
void	assign_spell_pointers(void);
int magic_resist(struct char_data *ch, struct char_data *victim);


/* FROM: utility.c
 */

int	number(int from, int to);
int	dice(int number, int size);
int  to_percentage(struct char_data *ch, int value);
int    get_mob_skill(struct char_data *mob, int nr);
char	*str_dup(const char *source);
int	str_cmp(const char *arg1, const char *arg2);
int	strn_cmp(const char *arg1, const char *arg2, int n);
void	log_death_trap(struct char_data *ch);
void	log(char *str);
void mudlog(char *str, char type, sbyte level, byte file);
void	sprintbit(long vektor, char *names[], char *result);
void	sprintflags(long vektor, char *result);
void	sprinttype(int type, char *names[], char *result);
int     get_line(FILE *fl, char *buf);
struct time_info_data real_time_passed(time_t t2, time_t t1);
struct time_info_data mud_time_passed(time_t t2, time_t t1);
struct time_info_data age(struct char_data *ch);
void	echo_off(int sock);
void	echo_on(int sock);
void CLASS_ABBR(struct char_data *ch, char *buf);
void format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen);
int replace_str(char **string, char *pattern, char *replacement, int rep_all, int max_size);
char   *stripcr(char *dest, const char *src);


/* FROM: weather.c
 */

void	weather_and_time(int mode);
void	another_hour(int mode);
void	weather_change(void);


/* FROM: ocs.c
 */

void print_room(struct char_data *ch, struct room_data *room);
void print_exit(struct char_data *ch, struct room_direction_data *dir);
void print_bits(char **vec, char *buf);
void print_sector_type(struct char_data *ch);
void get_sector_type(struct char_data *ch, struct room_data *room, int type);
void print_room_flags(struct char_data *ch, struct room_data *room);
void make_new_extra(struct extra_descr_data **descr);
void get_room_flags(struct char_data *ch, struct room_data *room, int type);
void print_extra_desc(struct char_data *ch, struct extra_descr_data **descr);
int check_extra_descr(struct extra_descr_data *descr);
void free_extra_descr(struct extra_descr_data **descr);
void change_string(char **old,char *new);
/* void adjust_reset_cmds(struct reset_com *com, sh_int rnum); */
void adjust_exits(sh_int rnum);
void adjust_crashrooms(sh_int rnum);
void adjust_chars(sh_int rnum);
void adjust_objs(sh_int rnum);
void adjust_zones(sh_int rnum);
void free_ex_descriptions(struct extra_descr_data *ls);
void free_room(struct room_data *room);
void move_roomblock(int from, int to);
void insert_room(struct room_data *room);
void room_cpy(struct room_data *to, struct room_data *from);
void ocs_main(struct char_data *ch, char *arg);
void save_room(FILE *fl, struct room_data *room);
int  ocs_save(int mode, int flag);


/* FROM: mobprog.c
 */
void  mprog_speech_trigger(char *txt, struct char_data *mob);
void  mprog_greet_trigger(struct char_data * ch);
void  mprog_entry_trigger(struct char_data * mob);
void  mprog_act_trigger(char *buf, struct char_data *mob, struct char_data *ch, struct obj_data *obj, void *vo);
void  mprog_read_programs(FILE * fp, struct index_data * pMobIndex);
void  mprog_hitprcnt_trigger(struct char_data * mob, struct char_data * ch);
void  mprog_death_trigger(struct char_data * mob, struct char_data * killer);
void  mprog_fight_trigger(struct char_data * mob, struct char_data * ch);
void  mprog_random_trigger(struct char_data * mob);
void  mprog_wordlist_check(char *arg, struct char_data * mob, struct char_data * actor, struct obj_data * obj, void *vo, int type);
void mprog_give_trigger(struct char_data *mob, struct char_data *ch, struct obj_data *obj);
void mprog_bribe_trigger(struct char_data *mob, struct char_data *ch, int amount);

/* FROM: mobcmd.c
 */
void perform_asound(struct char_data *ch, char *str);

/* FROM: random.c
 */
void circle_srandom(unsigned long initial_seed);
unsigned long circle_random(void);
