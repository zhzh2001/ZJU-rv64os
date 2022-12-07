#ifndef _DEFS_H
#define _DEFS_H

#define csr_read(csr)                        \
    ({                                       \
        register unsigned long __v;          \
        asm volatile("csrr %0, " #csr        \
                     : "=r"(__v)::"memory"); \
        __v;                                 \
    })

#define csr_write(csr, val)                       \
    ({                                            \
        unsigned long __v = (unsigned long)(val); \
        asm volatile("csrw " #csr ", %0"          \
                     :                            \
                     : "r"(__v)                   \
                     : "memory");                 \
    })

#define PHY_START 0x0000000080000000
#define PHY_SIZE 128 * 1024 * 1024 // 128MB， QEMU 默认内存大小
#define PHY_END (PHY_START + PHY_SIZE)

#define PGSIZE 0x1000 // 4KB
#define PGROUNDUP(addr) ((addr + PGSIZE - 1) & (~(PGSIZE - 1)))
#define PGROUNDDOWN(addr) (addr & (~(PGSIZE - 1)))

#define OPENSBI_SIZE (0x200000)

#define VM_START (0xffffffe000000000)
#define VM_END (0xffffffff00000000)
#define VM_SIZE (VM_END - VM_START)

#define PA2VA_OFFSET (VM_START - PHY_START)

#define VPN2(va) (((va) >> 30) & 0x1ff)
#define VPN1(va) (((va) >> 21) & 0x1ff)
#define VPN0(va) (((va) >> 12) & 0x1ff)
#define PGOFF(va) ((va)&0xfff)
#define MKPTE(pa, vrwx) ((((pa) >> 2) & 0x003ffffffffffc00) | (vrwx))
#define PTE2PA(pte) (((pte)&0x003ffffffffffc00) << 2)

#define PTE_V (1 << 0)
#define PTE_R (1 << 1)
#define PTE_W (1 << 2)
#define PTE_X (1 << 3)
#define PTE_U (1 << 4)

#define USER_START (0x0000000000000000) // user space start virtual address
#define USER_END (0x0000004000000000)   // user space end virtual address

#define STATUS_SUM (1 << 18)
#define STATUS_SPP (1 << 8)
#define STATUS_SPIE (1 << 5)

#endif
