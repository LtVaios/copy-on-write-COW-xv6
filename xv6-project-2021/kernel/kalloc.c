// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void* start_page_counters();
void freerange(void *pa_start, void *pa_end);

//Auth h diefthinsi tha einai h arxh tou pinaka anaforwn
extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
    uint64* pcounters;
    struct spinlock lock;
    struct run *freelist;
} kmem;

void
kinit() {
    //O pinakas akeraiwn me tis anafores opws to leei sthn ekfwnhsh
    void* init = start_page_counters();
    initlock(&kmem.lock, "kmem");
    freerange(init, (void*)PHYSTOP);
}

//Auth h synarthsh meiwnei ton metrhth twn diergasiwn pou vlepoun se mia selida
void
page_ref_dec(uint64 page) {
    //Vriskoume to katallhllo keli pinaka vriskontas ton arithmo selidas tou page kai afairontas thn teleftea desmeumenh thesh mnhmhs
    // (h teleftea thesh mnhmhs einai meta ton pinaka anaforwn pou exw, vlepe thn start_page_counters())
    //Meta diairoume me to PGSIZE gia na paroume thesh pinaka
    uint64 dec = --kmem.pcounters[(PGROUNDDOWN((uint64)page) - (uint64)end) / PGSIZE];

    //Edv an o metrhths ginei 0 tote shmainei oti kamia diergasia de xreiazetai pleon thn selida kai thn apodesmevoume
    if (dec == 0)
        kfree((void*) page);
}

//Auth h synarthsh auksanei ton metrhth
void
page_ref_inc(uint64 page) {
    kmem.pcounters[(PGROUNDDOWN((uint64)page) - (uint64)end) / PGSIZE]++;
}

void*
start_page_counters()
{
    uint64 i = 0;
    uint64 j = 0;
    //Vriskoume ton arithmo twn fysikwn selidwn gia na desmeusoume toses theseis ston pinaka
    //Kanoume kai swsth stroggylopoihsh gia na vgei pollapasio tou megethous ths selida
    uint64 all = (PGROUNDDOWN(PHYSTOP) - PGROUNDUP((uint64)end)) / PGSIZE;
    //Pairnoume thn prwth diefthinsi tou end se pollaplasio tou page number
    uint64 p = PGROUNDUP((uint64)end);

    //Xrhsimopoioume thn prwth diefthinsi meta ton kernel (end[]) gia na apothikeusoume ekei ton pinaka metrhshs anaforwn
    kmem.pcounters = (uint64*) p;

    //Phgainoume twra na vroume thn teleftea thesi meta ton pinaka mas gia na thn epistrepsoume
    while (i < all * sizeof(uint64)){
        p += PGSIZE;
        i += PGSIZE;
    }

    //Arxikopoihsh se 0 olwn twn keliwn
    while(j < all){
        kmem.pcounters[j] = 0;
        j++;
    }

    //epistrefoume thn teleftea thesi meta ton pinaka pou ftiaksame
    return (void*) p;
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  //Elegxoume an h selida pou theloume na kanoume free exei akoma anafores apo diergasies epanw ths
  //An exei panw apo 1 anafores shmainei oti kapoia diergasia teleiwse kai prospathei na thn kanei free
  //Omws den prepei na giei free ara meiwnoume ton counter kata 1 kai epistrefoume an o counter eksakolouthei na einai panw apo 0
  if (kmem.pcounters[(PGROUNDDOWN((uint64)pa) - (uint64)end) / PGSIZE] != 0)
      kmem.pcounters[(PGROUNDDOWN((uint64) pa) - (uint64) end) / PGSIZE] -= 1;

  if (kmem.pcounters[(PGROUNDDOWN((uint64)pa) - (uint64)end) / PGSIZE] != 0)
      return;

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
      kmem.freelist = r->next;
      //Edw arxikopoioume ton counter ths kainourias selidas me 1 (sigoura yparxei ston pinaka afou exoume desmeusei pinaka oso h fysikh mnhmh)
      kmem.pcounters[(PGROUNDDOWN((uint64)r) - (uint64)end) / PGSIZE] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
