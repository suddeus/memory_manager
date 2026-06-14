#pragma once

static const long PAGES_AMOUNT_FOR_EXTEND = 16;

static long mem_page_size = 0;
static long get_page_size();

const void* get_heap_top();

long extend_heap();