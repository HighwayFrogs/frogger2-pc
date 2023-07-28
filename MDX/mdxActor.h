#ifndef MDXACTOR_H_INCLUDED
#define MDXACTOR_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	float			radius;
	unsigned char	alpha;
	char			draw;
} MDX_ACTOR_SHADOW;

typedef struct TAG_MDX_ACTOR
{
	struct TAG_MDX_ACTOR		*next,*prev;
	struct TAG_MDX_ACTOR		*nextDraw,*prevDraw;
	unsigned long				listNum;

	float						distanceFromCamera;

	MDX_MATRIX					*matrix;
	char						status;
	short						xluOverride;
	int							flags;

	float						maxRadius;

	MDX_VECTOR					trueCentre;
	MDX_VECTOR					pos;
	MDX_VECTOR					oldpos;
	MDX_VECTOR					vel;
	MDX_VECTOR					rot;
	MDX_VECTOR					rotaim;
								
	MDX_QUATERNION				qRot;
												
	MDX_VECTOR	  				scale;

	MDX_OBJECT_CONTROLLER		*objectController;
	MDX_OBJECT_CONTROLLER		*LODObjectController;
	MDX_ACTOR_SHADOW			*shadow;
	MDX_ACTOR_ANIMATION			*animation;
	unsigned char				visible;

	MDX_TEXENTRY				*overrideTex;
	long						depthOff;

	long						draw;
	long						xform;

	float						radius;
} MDX_ACTOR;

#define	INIT_ANIMATION	(1 << 0)
#define	INIT_SHADOW		(1 << 1)

#define ACTOR_NOFLAGS		0
#define ACTOR_WRAPTC		(1<<0)
#define ACTOR_ALWAYSDRAW	(1<<1)
#define ACTOR_SLIDY			(1<<2)

extern MDX_ACTOR  *currentDrawActor;
extern MDX_VECTOR parentScaleStack[10];
extern int	  parentScaleStackLevel;
extern MDX_VECTOR *actorScale;
extern MDX_ACTOR *actorList;
extern void (*StartAnim)(MDX_ACTOR *me);

MDX_ACTOR *CreateActor(char *name, unsigned long flags);
void Animate(MDX_ACTOR *actor, int animNum, char loop, char queue, float speed);
void XformActor(MDX_ACTOR *actor,long v);
void DrawActor(MDX_ACTOR *actor);
MDX_OBJECT *FindActorSubObject(MDX_ACTOR *me, char *subName);
void AttachActorToSubobject(MDX_OBJECT *dest, MDX_ACTOR *src);
unsigned long CheckBoundingBox(MDX_VECTOR *bBox,MDX_MATRIX *m);
void InitAnims(MDX_ACTOR *tempActor);
void FreeUniqueActorList(void);
void FreeActor(MDX_ACTOR **toFree);

unsigned long AddActorToList(MDX_ACTOR *me, unsigned long listNum);
void ActorListDraw(long i);


#ifdef __cplusplus
}
#endif
#endif
