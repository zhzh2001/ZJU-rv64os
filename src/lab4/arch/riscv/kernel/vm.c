#include "vm.h"
#include "string.h"
#include "defs.h"
#include "mm.h"
/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long early_pgtbl[512] __attribute__((__aligned__(0x1000)));

void setup_vm(void)
{
	/*
	1. 由于是进行 1GB 的映射 这里不需要使用多级页表
	2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
		high bit 可以忽略
		中间9 bit 作为 early_pgtbl 的 index
		低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。
	3. Page Table Entry 的权限 V | R | W | X 位设置为 1
	*/
	memset(early_pgtbl, 0, sizeof(early_pgtbl));
	// map va 0x80000000 to pa 0x80000000
	early_pgtbl[VPN2(PHY_START)] = MKPTE(PHY_START, 0xf);
	// map va 0xffffffe000000000 to pa 0x80000000
	early_pgtbl[VPN2(VM_START)] = MKPTE(PHY_START, 0xf);
}

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

extern char _stext[], _srodata[], _sdata[];

void setup_vm_final(void)
{
	memset(swapper_pg_dir, 0x0, PGSIZE);

	// No OpenSBI mapping required

	// mapping kernel text X|-|R|V
	create_mapping(swapper_pg_dir, (uint64)_stext, (uint64)_stext - PA2VA_OFFSET, (uint64)_srodata - (uint64)_stext, 0xb);

	// mapping kernel rodata -|-|R|V
	create_mapping(swapper_pg_dir, (uint64)_srodata, (uint64)_srodata - PA2VA_OFFSET, (uint64)_sdata - (uint64)_srodata, 0x3);

	// mapping other memory -|W|R|V
	create_mapping(swapper_pg_dir, (uint64)_sdata, (uint64)_sdata - PA2VA_OFFSET, PHY_SIZE - ((uint64)_sdata - VM_START), 0x7);

	// set satp with swapper_pg_dir
	csr_write(satp, (((uint64)swapper_pg_dir - PA2VA_OFFSET) >> 12) | 0x8000000000000000);

	// flush TLB
	asm volatile("sfence.vma zero, zero");
	return;
}

/* 创建多级页表映射关系 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
{
	/*
	pgtbl 为根页表的基地址
	va, pa 为需要映射的虚拟地址、物理地址
	sz 为映射的大小
	perm 为映射的读写权限

	创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
	可以使用 V bit 来判断页表项是否存在
	*/
	for (uint64 i = 0; i < sz; i += PGSIZE)
	{
		uint64 vanow = va + i;
		uint64 panow = pa + i;
		uint64 vpn[3] = {VPN2(vanow), VPN1(vanow), VPN0(vanow)};
		uint64 *tbl = pgtbl;
		for (int j = 0; j < 2; j++)
		{
			uint64 *newtbl;
			if (!(tbl[vpn[j]] & 0x1))
			{
				newtbl = (uint64 *)kalloc();
				memset(newtbl, 0x0, PGSIZE);
				uint64 pa = (uint64)newtbl - PA2VA_OFFSET;
				tbl[vpn[j]] = MKPTE(pa, 0x1);
			}
			else
			{
				newtbl = (uint64 *)(PTE2PA(tbl[vpn[j]]) + PA2VA_OFFSET);
			}
			tbl = newtbl;
		}
		tbl[vpn[2]] = MKPTE(panow, perm);
	}
}