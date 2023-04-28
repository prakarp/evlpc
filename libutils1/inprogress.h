/* 
 * File:   inprogress.h
 * Author: pj
 *
 * Created on October 15, 2014, 7:47 PM
 */

#ifndef INPROGRESS_H
#define	INPROGRESS_H
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <cstdlib>

typedef struct {
    uint32_t magic;
    void *data;
    void *data2;
    void *data3;
    bool inuse;
} InProgress_t;

class InProgress {
    InProgress_t *inp;
    int m_n_items;
public:
    InProgress(int n_items = 100);
    ~InProgress();
    int alloc_slot();
    int return_slot(unsigned int i);
};

#endif	/* INPROGRESS_H */

