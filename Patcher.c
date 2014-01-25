#include "Patcher.h"

static BbQueue *	patch_item_list_new  (Patch *p);
static inline void 	patch_item_list_free (BbQueue *pil);

static PatchItem *	patch_item_new (Ztring *z, int offset);
static inline void 	patch_item_free (PatchItem *);
static void 		patch_item_debug (PatchItem *pi);

BbQueue *patch_list = NULL;

Patch *
patch_new (char *description, MemProc *mp, DWORD addr, unsigned char *code, unsigned char *signature, unsigned char *patch, char *mask)
{
	Patch *p;

	if ((p = malloc(sizeof(Patch))) == NULL)
		return NULL;

	// Add to the patches collection
	if (patch_list == NULL)
		patch_list = bb_queue_new();

	bb_queue_add(patch_list, p);

	// Init
	p->activated = FALSE;
	p->addr = addr;
	p->patch = patch;
	p->mask = mask;
	p->size = strlen(mask);
	p->ctxt = mp;
	p->description = strdup(description);
	p->signature = malloc(p->size);

	if (code == NULL)
		memcpy(p->signature, signature, p->size);
	else
		memcpy(p->signature, code, p->size);

	p->patch_items = patch_item_list_new(p);

	return p;
}

void
patch_set_activated (Patch *p, bool activated)
{
	if (!p)
	{
		warning("Patch is NULL");
		return;
	}

	if (!p->addr)
	{
		warning("%sPatch \"%s\" is not possible", (activated) ? "" : "Un", p->description);
		return;
	}

	if (activated)
	{
		if (p->activated)
		{
			warning("Patch \"%s\" is already activated", p->description);
			return;
		}

		PatchItem *pi;
		int success = 1;

		foreach_bbqueue_item (p->patch_items, pi)
		{
			char *str = ztring_get_text(pi->z);

			if (!write_to_memory(p->ctxt->proc, str, p->addr + pi->offset, ztring_get_len(pi->z)))
			{
				success = 0;
				warning("Patch \"%s\" : failure (0x%.8x)", p->description, p->addr);
				patch_item_debug(pi);
			}

			// Cleaning
			free(str);
		}

		if (success)
			debug("Patch \"%s\" : success (0x%.8x)", p->description, p->addr);
	}

	else
	{
		if (!p->activated)
			warning("Patch \"%s\" is already unactivated", p->description);

		// Restore the initial bytes
		if (write_to_memory(p->ctxt->proc, p->signature, p->addr, p->size))
			debug("UnPatch \"%s\" : success (0x%.8x)", p->description, p->addr);
		else
			warning("UnPatch \"%s\" : failure (0x%.8x)", p->description, p->addr);
	}

	p->activated = activated;
}

BbQueue *
patch_list_get ()
{
	BbQueue *res = patch_list;
	patch_list = NULL;
	return res;
}

void
patch_list_set (BbQueue *patchlist, bool enabled)
{
	if (patchlist == NULL)
		return;

	foreach_bbqueue_item (patchlist, Patch *p)
	{
		patch_set_activated(p, enabled);
	}
}

inline void
patch_list_free (BbQueue *pl)
{
	bb_queue_free_all (pl, patch_free);
}

void
patch_free (Patch *patch)
{
	if (patch != NULL)
	{
		free (patch->description);
		free (patch->signature);
		free (patch);
	}
}

void
patch_debug (Patch *p)
{
	debug (
		"\n--- Patch Debug : ---\n"
		" addr  = 0x%.8x\n"
		" size  = %d\n",
		p->addr,
		p->size
	);
}

// Patch Items list
static BbQueue *
patch_item_list_new (Patch *p)
{
	BbQueue *res = bb_queue_new();
	Ztring *z = ztring_new();

	for (int i = 0; i < p->size; i++)
	{
		if (p->mask[i] != '?')
			ztring_concat_letter(z, (unsigned char) p->patch[i]); // str += mask[i]

		else
		{
			if (ztring_get_len(z) != 0)
			{
				bb_queue_add(res, patch_item_new(z, i)); // res.add(str)
				z = ztring_new();
			}
		}
	}

	if (ztring_get_len(z) != 0)
		bb_queue_add(res, patch_item_new(z, p->size)); // res.add(str)

	return res;
}

static inline void
patch_item_list_free (BbQueue *pil)
{
	bb_queue_free_all (pil, patch_item_free);
}

// Patch Item
static PatchItem *
patch_item_new (Ztring *z, int offset)
{
	PatchItem *pi;

	if ((pi = malloc(sizeof(PatchItem))) == NULL)
		return NULL;

	pi->z = z;
	pi->offset = offset - ztring_get_len(z);

	return pi;
}

static inline void
patch_item_free (PatchItem *pi)
{
	if (pi != NULL)
	{
		free (pi);
	}
}

static void
patch_item_debug (PatchItem *pi)
{
	char buffer[1024 * 100] = {0};
	int loop = 0;
	unsigned char c;

	foreach_bbqueue_item_raw (pi->z->_text, c)
	{
		if (loop % 16 == 0)
			strcat(buffer, "\n\t");

		sprintf(buffer, "%s 0x%.2x", buffer, c);

		loop++;
	}

	debug (
		"\n--- PatchItem Debug : ---\n"
		" size = %d bytes\n"
		" offset = 0x%.8x\n"
		" data = [\n%s\n]",
		ztring_get_len(pi->z),
		pi->offset,
		buffer
	);
}
