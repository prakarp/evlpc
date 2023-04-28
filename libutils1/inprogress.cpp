#include "inprogress.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <cstdlib>

InProgress::InProgress(int n_items) {
    m_n_items = n_items;
    inp = (InProgress_t *) malloc(m_n_items * sizeof (InProgress_t));
    assert(inp != NULL);
}

InProgress::~InProgress() {
    memset((char *)inp, 0, m_n_items * sizeof (InProgress_t));
    free(inp);
    inp = NULL;
    m_n_items = 0;
}

int InProgress::alloc_slot() {
    int i;
    for (i = 0; i < m_n_items; i++) {
        if (!inp[i].inuse) {
            inp[i].inuse = true;
            inp[i].data = NULL;
            inp[i].data2 = NULL;
            inp[i].magic = rand();
            return i;
        }
    }
    return -1;
}

int InProgress::return_slot(unsigned int i) {
    if (i >= m_n_items) {
        return -1;
    }
    inp[i].data = NULL;
    inp[i].data2 = NULL;
    inp[i].magic = 0;
    if (!inp[i].inuse) {
        inp[i].inuse = false;
        return -1;
    }
    inp[i].inuse = false;
    return 0;
}
