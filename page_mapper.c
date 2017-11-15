/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   page_mapper.c
 * Author: root
 *
 * Created on November 15, 2017, 12:40 AM
 */

#include <linux/module.h>
//#include <linux/slab.h>


#include "./utils/log.h"

#include "page_mapper.h"

///  kernel module functions
/*
static struct wm_operations_struct wm_driver =
{
    .name = DRIVER_NAME,
    .id_table = p_ids,
    .probe = probe,
    .remove = remove,
};
*/
/**
 * @brief Register kernel module. Kernel call's it on insmod
 *
 * @return 0 on success
 *	   -1 error
 */
static int mod_init(void)
{

    LOG("Module loaded\n");

    return 0;
}

/**
 * @brief Unregister kernel module. Kernel call's it on rmmod
 *
 * @return void
 */
static void mod_exit(void)
{

    LOG("Module unloaded\n");
}


MODULE_LICENSE("GPL");

module_init(mod_init);
module_exit(mod_exit);