#include "global.h"
#include "battle.h"
#include "pokemon.h"
#include "pokemon_animation.h"
#include "sprite.h"
#include "task.h"
#include "trig.h"
#include "util.h"
#include "data.h"
#include "constants/battle_anim.h"
#include "constants/rgb.h"

/*
    This file handles the movements of the Pokémon intro animations.

    Each animation type is identified by an ANIM_* constant that
    refers to a sprite callback to start the animation. These functions
    are named Anim_<name> or Anim_<name>_<variant>. Many of these
    functions share additional movement functions to do a variation of the
    same movement (e.g. a faster or larger movement).
    Vertical and Horizontal are frequently shortened to V and H.

    Every front animation uses 1 of these ANIMs, and every back animation
    uses a BACK_ANIM_* that refers to a set of 3 ANIM functions. Which of the
    3 that gets used depends on the Pokémon's nature (see sBackAnimationIds).

    The table linking species to a BACK_ANIM is in this file (sSpeciesToBackAnimSet)
    and the table linking species to an ANIM for their front animation (sMonFrontAnimIdsTable)
    is now ALSO in this file. (It was previously in pokemon.c - WHY???)

    These are the functions that will start an animation:
    - LaunchAnimationTaskForFrontSprite
    - LaunchAnimationTaskForBackSprite
    - StartMonSummaryAnimation
*/

#define sDontFlip data[1]  // TRUE if a normal animation, FALSE if Summary Screen animation

struct PokemonAnimData
{
    u16 delay;
    s16 speed; // Only used by 2 sets of animations
    s16 runs; // Number of times to do the animation
    s16 rotation;
    s16 data; // General use
};

struct YellowFlashData
{
    bool8 isYellow;
    u8 time;
};

static void Anim_VerticalSquishBounce(struct Sprite *sprite);
static void Anim_CircularStretchTwice(struct Sprite *sprite);
static void Anim_HorizontalVibrate(struct Sprite *sprite);
static void Anim_HorizontalSlide(struct Sprite *sprite);
static void Anim_VerticalSlide(struct Sprite *sprite);
static void Anim_BounceRotateToSides(struct Sprite *sprite);
static void Anim_VerticalJumpsHorizontalJumps(struct Sprite *sprite);
static void Anim_RotateToSides(struct Sprite *sprite);
static void Anim_RotateToSides_Twice(struct Sprite *sprite);
static void Anim_GrowVibrate(struct Sprite *sprite);
static void Anim_ZigzagFast(struct Sprite *sprite);
static void Anim_SwingConcave(struct Sprite *sprite);
static void Anim_SwingConcave_Fast(struct Sprite *sprite);
static void Anim_SwingConvex(struct Sprite *sprite);
static void Anim_SwingConvex_Fast(struct Sprite *sprite);
static void Anim_HorizontalShake(struct Sprite *sprite);
static void Anim_VerticalShake(struct Sprite *sprite);
static void Anim_CircularVibrate(struct Sprite *sprite);
static void Anim_Twist(struct Sprite *sprite);
static void Anim_ShrinkGrow(struct Sprite *sprite);
static void Anim_CircleCounterclockwise(struct Sprite *sprite);
static void Anim_GlowBlack(struct Sprite *sprite);
static void Anim_HorizontalStretch(struct Sprite *sprite);
static void Anim_VerticalStretch(struct Sprite *sprite);
static void Anim_RisingWobble(struct Sprite *sprite);
static void Anim_VerticalShakeTwice(struct Sprite *sprite);
static void Anim_TipMoveForward(struct Sprite *sprite);
static void Anim_HorizontalPivot(struct Sprite *sprite);
static void Anim_VerticalSlideWobble(struct Sprite *sprite);
static void Anim_HorizontalSlideWobble(struct Sprite *sprite);
static void Anim_VerticalJumps_Big(struct Sprite *sprite);
static void Anim_Spin_Long(struct Sprite *sprite);
static void Anim_GlowOrange(struct Sprite *sprite);
static void Anim_GlowRed(struct Sprite *sprite);
static void Anim_GlowBlue(struct Sprite *sprite);
static void Anim_GlowYellow(struct Sprite *sprite);
static void Anim_GlowPurple(struct Sprite *sprite);
static void Anim_BackAndLunge(struct Sprite *sprite);
static void Anim_BackFlip(struct Sprite *sprite);
static void Anim_Flicker(struct Sprite *sprite);
static void Anim_BackFlipBig(struct Sprite *sprite);
static void Anim_FrontFlip(struct Sprite *sprite);
static void Anim_TumblingFrontFlip(struct Sprite *sprite);
static void Anim_Figure8(struct Sprite *sprite);
static void Anim_FlashYellow(struct Sprite *sprite);
static void Anim_SwingConcave_FastShort(struct Sprite *sprite);
static void Anim_SwingConvex_FastShort(struct Sprite *sprite);
static void Anim_RotateUpSlamDown(struct Sprite *sprite);
static void Anim_DeepVerticalSquishBounce(struct Sprite *sprite);
static void Anim_HorizontalJumps(struct Sprite *sprite);
static void Anim_HorizontalJumpsVerticalStretch(struct Sprite *sprite);
static void Anim_RotateToSides_Fast(struct Sprite *sprite);
static void Anim_RotateUpToSides(struct Sprite *sprite);
static void Anim_FlickerIncreasing(struct Sprite *sprite);
static void Anim_TipHopForward(struct Sprite *sprite);
static void Anim_PivotShake(struct Sprite *sprite);
static void Anim_TipAndShake(struct Sprite *sprite);
static void Anim_VibrateToCorners(struct Sprite *sprite);
static void Anim_GrowInStages(struct Sprite *sprite);
static void Anim_VerticalSpring(struct Sprite *sprite);
static void Anim_VerticalRepeatedSpring(struct Sprite *sprite);
static void Anim_SpringRising(struct Sprite *sprite);
static void Anim_HorizontalSpring(struct Sprite *sprite);
static void Anim_HorizontalRepeatedSpring_Slow(struct Sprite *sprite);
static void Anim_HorizontalSlideShrink(struct Sprite *sprite);
static void Anim_LungeGrow(struct Sprite *sprite);
static void Anim_CircleIntoBackground(struct Sprite *sprite);
static void Anim_RapidHorizontalHops(struct Sprite *sprite);
static void Anim_FourPetal(struct Sprite *sprite);
static void Anim_VerticalSquishBounce_Slow(struct Sprite *sprite);
static void Anim_HorizontalSlide_Slow(struct Sprite *sprite);
static void Anim_VerticalSlide_Slow(struct Sprite *sprite);
static void Anim_BounceRotateToSides_Small(struct Sprite *sprite);
static void Anim_BounceRotateToSides_Slow(struct Sprite *sprite);
static void Anim_BounceRotateToSides_SmallSlow(struct Sprite *sprite);
static void Anim_ZigzagSlow(struct Sprite *sprite);
static void Anim_HorizontalShake_Slow(struct Sprite *sprite);
static void Anim_VertialShake_Slow(struct Sprite *sprite);
static void Anim_Twist_Twice(struct Sprite *sprite);
static void Anim_CircleCounterclockwise_Slow(struct Sprite *sprite);
static void Anim_VerticalShakeTwice_Slow(struct Sprite *sprite);
static void Anim_VerticalSlideWobble_Small(struct Sprite *sprite);
static void Anim_VerticalJumps_Small(struct Sprite *sprite);
static void Anim_Spin(struct Sprite *sprite);
static void Anim_TumblingFrontFlip_Twice(struct Sprite *sprite);
static void Anim_DeepVerticalSquishBounce_Twice(struct Sprite *sprite);
static void Anim_HorizontalJumpsVerticalStretch_Twice(struct Sprite *sprite);
static void Anim_VerticalShakeBack(struct Sprite *sprite);
static void Anim_VerticalShakeBack_Slow(struct Sprite *sprite);
static void Anim_VerticalShakeHorizontalSlide_Slow(struct Sprite *sprite);
static void Anim_VerticalStretchBothEnds_Slow(struct Sprite *sprite);
static void Anim_HorizontalStretchFar_Slow(struct Sprite *sprite);
static void Anim_VerticalShakeLowTwice(struct Sprite *sprite);
static void Anim_HorizontalShake_Fast(struct Sprite *sprite);
static void Anim_HorizontalSlide_Fast(struct Sprite *sprite);
static void Anim_HorizontalVibrate_Fast(struct Sprite *sprite);
static void Anim_HorizontalVibrate_Fastest(struct Sprite *sprite);
static void Anim_VerticalShakeBack_Fast(struct Sprite *sprite);
static void Anim_VerticalShakeLowTwice_Slow(struct Sprite *sprite);
static void Anim_VerticalShakeLowTwice_Fast(struct Sprite *sprite);
static void Anim_CircleCounterclockwise_Long(struct Sprite *sprite);
static void Anim_GrowStutter_Slow(struct Sprite *sprite);
static void Anim_VerticalShakeHorizontalSlide(struct Sprite *sprite);
static void Anim_VerticalShakeHorizontalSlide_Fast(struct Sprite *sprite);
static void Anim_TriangleDown_Slow(struct Sprite *sprite);
static void Anim_TriangleDown(struct Sprite *sprite);
static void Anim_TriangleDown_Fast(struct Sprite *sprite);
static void Anim_Grow(struct Sprite *sprite);
static void Anim_Grow_Twice(struct Sprite *sprite);
static void Anim_HorizontalSpring_Fast(struct Sprite *sprite);
static void Anim_HorizontalSpring_Slow(struct Sprite *sprite);
static void Anim_HorizontalRepeatedSpring_Fast(struct Sprite *sprite);
static void Anim_HorizontalRepeatedSpring(struct Sprite *sprite);
static void Anim_ShrinkGrow_Fast(struct Sprite *sprite);
static void Anim_ShrinkGrow_Slow(struct Sprite *sprite);
static void Anim_VerticalStretchBothEnds(struct Sprite *sprite);
static void Anim_VerticalStretchBothEnds_Twice(struct Sprite *sprite);
static void Anim_HorizontalStretchFar_Twice(struct Sprite *sprite);
static void Anim_HorizontalStretchFar(struct Sprite *sprite);
static void Anim_GrowStutter_Twice(struct Sprite *sprite);
static void Anim_GrowStutter(struct Sprite *sprite);
static void Anim_ConcaveArcLarge_Slow(struct Sprite *sprite);
static void Anim_ConcaveArcLarge(struct Sprite *sprite);
static void Anim_ConcaveArcLarge_Twice(struct Sprite *sprite);
static void Anim_ConvexDoubleArc_Slow(struct Sprite *sprite);
static void Anim_ConvexDoubleArc(struct Sprite *sprite);
static void Anim_ConvexDoubleArc_Twice(struct Sprite *sprite);
static void Anim_ConcaveArcSmall_Slow(struct Sprite *sprite);
static void Anim_ConcaveArcSmall(struct Sprite *sprite);
static void Anim_ConcaveArcSmall_Twice(struct Sprite *sprite);
static void Anim_HorizontalDip(struct Sprite *sprite);
static void Anim_HorizontalDip_Fast(struct Sprite *sprite);
static void Anim_HorizontalDip_Twice(struct Sprite *sprite);
static void Anim_ShrinkGrowVibrate_Fast(struct Sprite *sprite);
static void Anim_ShrinkGrowVibrate(struct Sprite *sprite);
static void Anim_ShrinkGrowVibrate_Slow(struct Sprite *sprite);
static void Anim_JoltRight_Fast(struct Sprite *sprite);
static void Anim_JoltRight(struct Sprite *sprite);
static void Anim_JoltRight_Slow(struct Sprite *sprite);
static void Anim_ShakeFlashYellow_Fast(struct Sprite *sprite);
static void Anim_ShakeFlashYellow(struct Sprite *sprite);
static void Anim_ShakeFlashYellow_Slow(struct Sprite *sprite);
static void Anim_ShakeGlowRed_Fast(struct Sprite *sprite);
static void Anim_ShakeGlowRed(struct Sprite *sprite);
static void Anim_ShakeGlowRed_Slow(struct Sprite *sprite);
static void Anim_ShakeGlowGreen_Fast(struct Sprite *sprite);
static void Anim_ShakeGlowGreen(struct Sprite *sprite);
static void Anim_ShakeGlowGreen_Slow(struct Sprite *sprite);
static void Anim_ShakeGlowBlue_Fast(struct Sprite *sprite);
static void Anim_ShakeGlowBlue(struct Sprite *sprite);
static void Anim_ShakeGlowBlue_Slow(struct Sprite *sprite);
static void Anim_ShakeGlowBlack_Slow(struct Sprite *sprite);
static void Anim_ShakeGlowWhite_Slow(struct Sprite *sprite);
static void Anim_ShakeGlowPurple_Slow(struct Sprite *sprite);

static void WaitAnimEnd(struct Sprite *sprite);

static struct PokemonAnimData sAnims[MAX_BATTLERS_COUNT];
static u8 sAnimIdx;
static bool32 sIsSummaryAnim;

static const u8 sSpeciesToBackAnimSet[NUM_SPECIES] =
{
    [SPECIES_BULBASAUR]  = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_IVYSAUR]    = BACK_ANIM_H_SLIDE,
    [SPECIES_VENUSAUR]   = BACK_ANIM_H_SHAKE,
    [SPECIES_CHARMANDER] = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_CHARMELEON] = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_CHARIZARD]  = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_SQUIRTLE]   = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_WARTORTLE]  = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_BLASTOISE]  = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_CATERPIE]   = BACK_ANIM_H_SLIDE,
    [SPECIES_METAPOD]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_BUTTERFREE] = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_WEEDLE]     = BACK_ANIM_H_SLIDE,
    [SPECIES_KAKUNA]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_BEEDRILL]   = BACK_ANIM_H_VIBRATE,
    [SPECIES_PIDGEY]     = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_PIDGEOTTO]  = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_PIDGEOT]    = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_RATTATA]    = BACK_ANIM_V_SHAKE_H_SLIDE,
    [SPECIES_RATICATE]   = BACK_ANIM_V_SHAKE_H_SLIDE,
    [SPECIES_SPEAROW]    = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_FEAROW]     = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_EKANS]      = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_ARBOK]      = BACK_ANIM_V_SHAKE,
    [SPECIES_PIKACHU]    = BACK_ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_RAICHU]     = BACK_ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_SANDSHREW]  = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_SANDSLASH]  = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_NIDORAN_F]  = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_NIDORINA]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_NIDOQUEEN]  = BACK_ANIM_V_SHAKE,
    [SPECIES_NIDORAN_M]  = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_NIDORINO]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_NIDOKING]   = BACK_ANIM_V_SHAKE,
    [SPECIES_CLEFAIRY]   = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_CLEFABLE]   = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_VULPIX]     = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_NINETALES]  = BACK_ANIM_H_VIBRATE,
    [SPECIES_JIGGLYPUFF] = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_WIGGLYTUFF] = BACK_ANIM_GROW,
    [SPECIES_ZUBAT]      = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_GOLBAT]     = BACK_ANIM_V_SHAKE,
    [SPECIES_ODDISH]     = BACK_ANIM_H_SLIDE,
    [SPECIES_GLOOM]      = BACK_ANIM_H_SLIDE,
    [SPECIES_VILEPLUME]  = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_PARAS]      = BACK_ANIM_H_SLIDE,
    [SPECIES_PARASECT]   = BACK_ANIM_H_SHAKE,
    [SPECIES_VENONAT]    = BACK_ANIM_V_SHAKE_H_SLIDE,
    [SPECIES_VENOMOTH]   = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_DIGLETT]    = BACK_ANIM_V_SHAKE,
    [SPECIES_DUGTRIO]    = BACK_ANIM_V_SHAKE,
    [SPECIES_MEOWTH]     = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_PERSIAN]    = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_PSYDUCK]    = BACK_ANIM_H_SLIDE,
    [SPECIES_GOLDUCK]    = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_MANKEY]     = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_PRIMEAPE]   = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_GROWLITHE]  = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_ARCANINE]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_POLIWAG]    = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_POLIWHIRL]  = BACK_ANIM_V_SHAKE,
    [SPECIES_POLIWRATH]  = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_ABRA]       = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_KADABRA]    = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_ALAKAZAM]   = BACK_ANIM_GROW_STUTTER,
    [SPECIES_MACHOP]     = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_MACHOKE]    = BACK_ANIM_V_SHAKE,
    [SPECIES_MACHAMP]    = BACK_ANIM_V_SHAKE,
    [SPECIES_BELLSPROUT] = BACK_ANIM_V_STRETCH,
    [SPECIES_WEEPINBELL] = BACK_ANIM_V_STRETCH,
    [SPECIES_VICTREEBEL] = BACK_ANIM_V_STRETCH,
    [SPECIES_TENTACOOL]  = BACK_ANIM_H_SLIDE,
    [SPECIES_TENTACRUEL] = BACK_ANIM_H_SLIDE,
    [SPECIES_GEODUDE]    = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_GRAVELER]   = BACK_ANIM_H_SHAKE,
    [SPECIES_GOLEM]      = BACK_ANIM_H_SHAKE,
    [SPECIES_PONYTA]     = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_RAPIDASH]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_SLOWPOKE]   = BACK_ANIM_H_SLIDE,
    [SPECIES_SLOWBRO]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_MAGNEMITE]  = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_MAGNETON]   = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_FARFETCHD]  = BACK_ANIM_H_SLIDE,
    [SPECIES_DODUO]      = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_DODRIO]     = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_SEEL]       = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_DEWGONG]    = BACK_ANIM_H_SLIDE,
    [SPECIES_GRIMER]     = BACK_ANIM_V_STRETCH,
    [SPECIES_MUK]        = BACK_ANIM_H_STRETCH,
    [SPECIES_SHELLDER]   = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_CLOYSTER]   = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_GASTLY]     = BACK_ANIM_H_VIBRATE,
    [SPECIES_HAUNTER]    = BACK_ANIM_H_VIBRATE,
    [SPECIES_GENGAR]     = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_ONIX]       = BACK_ANIM_V_SHAKE,
    [SPECIES_DROWZEE]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_HYPNO]      = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_KRABBY]     = BACK_ANIM_V_SHAKE_H_SLIDE,
    [SPECIES_KINGLER]    = BACK_ANIM_V_SHAKE,
    [SPECIES_VOLTORB]    = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_ELECTRODE]  = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_EXEGGCUTE]  = BACK_ANIM_H_SLIDE,
    [SPECIES_EXEGGUTOR]  = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_CUBONE]     = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_MAROWAK]    = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_HITMONLEE]  = BACK_ANIM_H_SLIDE,
    [SPECIES_HITMONCHAN] = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_LICKITUNG]  = BACK_ANIM_H_SLIDE,
    [SPECIES_KOFFING]    = BACK_ANIM_GROW,
    [SPECIES_WEEZING]    = BACK_ANIM_GROW,
    [SPECIES_RHYHORN]    = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_RHYDON]     = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_CHANSEY]    = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_TANGELA]    = BACK_ANIM_V_STRETCH,
    [SPECIES_KANGASKHAN] = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_HORSEA]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_SEADRA]     = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_GOLDEEN]    = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_SEAKING]    = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_STARYU]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_STARMIE]    = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_MR_MIME]    = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_SCYTHER]    = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_JYNX]       = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_ELECTABUZZ] = BACK_ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_MAGMAR]     = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_PINSIR]     = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_TAUROS]     = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_MAGIKARP]   = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_GYARADOS]   = BACK_ANIM_V_SHAKE,
    [SPECIES_LAPRAS]     = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_DITTO]      = BACK_ANIM_SHRINK_GROW,
    [SPECIES_EEVEE]      = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_VAPOREON]   = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_JOLTEON]    = BACK_ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_FLAREON]    = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_PORYGON]    = BACK_ANIM_H_VIBRATE,
    [SPECIES_OMANYTE]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_OMASTAR]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_KABUTO]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_KABUTOPS]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_AERODACTYL] = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_SNORLAX]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_ARTICUNO]   = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_ZAPDOS]     = BACK_ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_MOLTRES]    = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_DRATINI]    = BACK_ANIM_H_SLIDE,
    [SPECIES_DRAGONAIR]  = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_DRAGONITE]  = BACK_ANIM_V_SHAKE,
    [SPECIES_MEWTWO]     = BACK_ANIM_GROW_STUTTER,
    [SPECIES_MEW]        = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_CHIKORITA]  = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_BAYLEEF]    = BACK_ANIM_H_SLIDE,
    [SPECIES_MEGANIUM]   = BACK_ANIM_V_SHAKE,
    [SPECIES_CYNDAQUIL]  = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_QUILAVA]    = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_TYPHLOSION] = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_TOTODILE]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_CROCONAW]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_FERALIGATR] = BACK_ANIM_V_SHAKE,
    [SPECIES_SENTRET]    = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_FURRET]     = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_HOOTHOOT]   = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_NOCTOWL]    = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_LEDYBA]     = BACK_ANIM_V_SHAKE_H_SLIDE,
    [SPECIES_LEDIAN]     = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_SPINARAK]   = BACK_ANIM_V_SHAKE_H_SLIDE,
    [SPECIES_ARIADOS]    = BACK_ANIM_H_SLIDE,
    [SPECIES_CROBAT]     = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_CHINCHOU]   = BACK_ANIM_V_STRETCH,
    [SPECIES_LANTURN]    = BACK_ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_PICHU]      = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_CLEFFA]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_IGGLYBUFF]  = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_TOGEPI]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_TOGETIC]    = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_NATU]       = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_XATU]       = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_MAREEP]     = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_FLAAFFY]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_AMPHAROS]   = BACK_ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_BELLOSSOM]  = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_MARILL]     = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_AZUMARILL]  = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_SUDOWOODO]  = BACK_ANIM_H_SLIDE,
    [SPECIES_POLITOED]   = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_HOPPIP]     = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_SKIPLOOM]   = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_JUMPLUFF]   = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_AIPOM]      = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_SUNKERN]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_SUNFLORA]   = BACK_ANIM_H_SLIDE,
    [SPECIES_YANMA]      = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_WOOPER]     = BACK_ANIM_V_STRETCH,
    [SPECIES_QUAGSIRE]   = BACK_ANIM_H_SLIDE,
    [SPECIES_ESPEON]     = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_UMBREON]    = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_MURKROW]    = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_SLOWKING]   = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_MISDREAVUS] = BACK_ANIM_H_VIBRATE,
    [SPECIES_UNOWN]      = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_WOBBUFFET]  = BACK_ANIM_V_STRETCH,
    [SPECIES_GIRAFARIG]  = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_PINECO]     = BACK_ANIM_H_SHAKE,
    [SPECIES_FORRETRESS] = BACK_ANIM_V_SHAKE,
    [SPECIES_DUNSPARCE]  = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_GLIGAR]     = BACK_ANIM_SHRINK_GROW,
    [SPECIES_STEELIX]    = BACK_ANIM_V_SHAKE,
    [SPECIES_SNUBBULL]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_GRANBULL]   = BACK_ANIM_V_SHAKE,
    [SPECIES_QWILFISH]   = BACK_ANIM_GROW_STUTTER,
    [SPECIES_SCIZOR]     = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_SHUCKLE]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_HERACROSS]  = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_SNEASEL]    = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_TEDDIURSA]  = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_URSARING]   = BACK_ANIM_V_SHAKE,
    [SPECIES_SLUGMA]     = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_MAGCARGO]   = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_SWINUB]     = BACK_ANIM_V_SHAKE_H_SLIDE,
    [SPECIES_PILOSWINE]  = BACK_ANIM_H_SHAKE,
    [SPECIES_CORSOLA]    = BACK_ANIM_H_SLIDE,
    [SPECIES_REMORAID]   = BACK_ANIM_H_SLIDE,
    [SPECIES_OCTILLERY]  = BACK_ANIM_SHRINK_GROW,
    [SPECIES_DELIBIRD]   = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_MANTINE]    = BACK_ANIM_H_SLIDE,
    [SPECIES_SKARMORY]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_HOUNDOUR]   = BACK_ANIM_V_SHAKE,
    [SPECIES_HOUNDOOM]   = BACK_ANIM_V_SHAKE,
    [SPECIES_KINGDRA]    = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_PHANPY]     = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_DONPHAN]    = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_PORYGON2]   = BACK_ANIM_H_VIBRATE,
    [SPECIES_STANTLER]   = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_SMEARGLE]   = BACK_ANIM_H_SLIDE,
    [SPECIES_TYROGUE]    = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_HITMONTOP]  = BACK_ANIM_CIRCLE_COUNTERCLOCKWISE,
    [SPECIES_SMOOCHUM]   = BACK_ANIM_H_SLIDE,
    [SPECIES_ELEKID]     = BACK_ANIM_H_SHAKE,
    [SPECIES_MAGBY]      = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_MILTANK]    = BACK_ANIM_H_SLIDE,
    [SPECIES_BLISSEY]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_RAIKOU]     = BACK_ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_ENTEI]      = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_SUICUNE]    = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_LARVITAR]   = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_PUPITAR]    = BACK_ANIM_V_SHAKE,
    [SPECIES_TYRANITAR]  = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_LUGIA]      = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_HO_OH]      = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_CELEBI]     = BACK_ANIM_SHAKE_GLOW_GREEN,
    [SPECIES_TREECKO]    = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_GROVYLE]    = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_SCEPTILE]   = BACK_ANIM_V_SHAKE,
    [SPECIES_TORCHIC]    = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_COMBUSKEN]  = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_BLAZIKEN]   = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_MUDKIP]     = BACK_ANIM_H_SLIDE,
    [SPECIES_MARSHTOMP]  = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_SWAMPERT]   = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_POOCHYENA]  = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_MIGHTYENA]  = BACK_ANIM_H_SHAKE,
    [SPECIES_ZIGZAGOON]  = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_LINOONE]    = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_WURMPLE]    = BACK_ANIM_H_SHAKE,
    [SPECIES_SILCOON]    = BACK_ANIM_H_SHAKE,
    [SPECIES_BEAUTIFLY]  = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_CASCOON]    = BACK_ANIM_V_STRETCH,
    [SPECIES_DUSTOX]     = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_LOTAD]      = BACK_ANIM_H_SLIDE,
    [SPECIES_LOMBRE]     = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_LUDICOLO]   = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_SEEDOT]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_NUZLEAF]    = BACK_ANIM_V_SHAKE,
    [SPECIES_SHIFTRY]    = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_NINCADA]    = BACK_ANIM_V_SHAKE_H_SLIDE,
    [SPECIES_NINJASK]    = BACK_ANIM_H_VIBRATE,
    [SPECIES_SHEDINJA]   = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_TAILLOW]    = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_SWELLOW]    = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_SHROOMISH]  = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_BRELOOM]    = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_SPINDA]     = BACK_ANIM_CIRCLE_COUNTERCLOCKWISE,
    [SPECIES_WINGULL]    = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_PELIPPER]   = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_SURSKIT]    = BACK_ANIM_H_SPRING,
    [SPECIES_MASQUERAIN] = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_WAILMER]    = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_WAILORD]    = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_SKITTY]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_DELCATTY]   = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_KECLEON]    = BACK_ANIM_H_VIBRATE,
    [SPECIES_BALTOY]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_CLAYDOL]    = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_NOSEPASS]   = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_TORKOAL]    = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_SABLEYE]    = BACK_ANIM_H_VIBRATE,
    [SPECIES_BARBOACH]   = BACK_ANIM_V_STRETCH,
    [SPECIES_WHISCASH]   = BACK_ANIM_V_SHAKE,
    [SPECIES_LUVDISC]    = BACK_ANIM_H_SPRING_REPEATED,
    [SPECIES_CORPHISH]   = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_CRAWDAUNT]  = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_FEEBAS]     = BACK_ANIM_H_SPRING,
    [SPECIES_MILOTIC]    = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_CARVANHA]   = BACK_ANIM_H_SPRING_REPEATED,
    [SPECIES_SHARPEDO]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_TRAPINCH]   = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_VIBRAVA]    = BACK_ANIM_H_VIBRATE,
    [SPECIES_FLYGON]     = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_MAKUHITA]   = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_HARIYAMA]   = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_ELECTRIKE]  = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_MANECTRIC]  = BACK_ANIM_V_SHAKE,
    [SPECIES_NUMEL]      = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_CAMERUPT]   = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_SPHEAL]     = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_SEALEO]     = BACK_ANIM_V_SHAKE,
    [SPECIES_WALREIN]    = BACK_ANIM_V_SHAKE,
    [SPECIES_CACNEA]     = BACK_ANIM_V_SHAKE_H_SLIDE,
    [SPECIES_CACTURNE]   = BACK_ANIM_H_SHAKE,
    [SPECIES_SNORUNT]    = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_GLALIE]     = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_LUNATONE]   = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_SOLROCK]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_AZURILL]    = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_SPOINK]     = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_GRUMPIG]    = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_PLUSLE]     = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_MINUN]      = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_MAWILE]     = BACK_ANIM_V_SHAKE,
    [SPECIES_MEDITITE]   = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_MEDICHAM]   = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_SWABLU]     = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_ALTARIA]    = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_WYNAUT]     = BACK_ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_DUSKULL]    = BACK_ANIM_H_VIBRATE,
    [SPECIES_DUSCLOPS]   = BACK_ANIM_H_VIBRATE,
    [SPECIES_ROSELIA]    = BACK_ANIM_SHAKE_GLOW_GREEN,
    [SPECIES_SLAKOTH]    = BACK_ANIM_H_SLIDE,
    [SPECIES_VIGOROTH]   = BACK_ANIM_CONCAVE_ARC_LARGE,
    [SPECIES_SLAKING]    = BACK_ANIM_H_SHAKE,
    [SPECIES_GULPIN]     = BACK_ANIM_V_STRETCH,
    [SPECIES_SWALOT]     = BACK_ANIM_V_STRETCH,
    [SPECIES_TROPIUS]    = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_WHISMUR]    = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_LOUDRED]    = BACK_ANIM_V_SHAKE,
    [SPECIES_EXPLOUD]    = BACK_ANIM_GROW_STUTTER,
    [SPECIES_CLAMPERL]   = BACK_ANIM_DIP_RIGHT_SIDE,
    [SPECIES_HUNTAIL]    = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_GOREBYSS]   = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_ABSOL]      = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_SHUPPET]    = BACK_ANIM_H_VIBRATE,
    [SPECIES_BANETTE]    = BACK_ANIM_H_VIBRATE,
    [SPECIES_SEVIPER]    = BACK_ANIM_V_STRETCH,
    [SPECIES_ZANGOOSE]   = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_RELICANTH]  = BACK_ANIM_H_SLIDE,
    [SPECIES_ARON]       = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_LAIRON]     = BACK_ANIM_V_SHAKE,
    [SPECIES_AGGRON]     = BACK_ANIM_V_SHAKE_LOW,
    [SPECIES_CASTFORM]   = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_VOLBEAT]    = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_ILLUMISE]   = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_LILEEP]     = BACK_ANIM_H_STRETCH,
    [SPECIES_CRADILY]    = BACK_ANIM_V_STRETCH,
    [SPECIES_ANORITH]    = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_ARMALDO]    = BACK_ANIM_V_SHAKE,
    [SPECIES_RALTS]      = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_KIRLIA]     = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_GARDEVOIR]  = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_BAGON]      = BACK_ANIM_V_SHAKE,
    [SPECIES_SHELGON]    = BACK_ANIM_V_SHAKE,
    [SPECIES_SALAMENCE]  = BACK_ANIM_H_SHAKE,
    [SPECIES_BELDUM]     = BACK_ANIM_TRIANGLE_DOWN,
    [SPECIES_METANG]     = BACK_ANIM_JOLT_RIGHT,
    [SPECIES_METAGROSS]  = BACK_ANIM_V_SHAKE,
    [SPECIES_REGIROCK]   = BACK_ANIM_V_SHAKE,
    [SPECIES_REGICE]     = BACK_ANIM_V_SHAKE,
    [SPECIES_REGISTEEL]  = BACK_ANIM_V_SHAKE,
    [SPECIES_KYOGRE]     = BACK_ANIM_SHAKE_GLOW_BLUE,
    [SPECIES_GROUDON]    = BACK_ANIM_SHAKE_GLOW_RED,
    [SPECIES_RAYQUAZA]   = BACK_ANIM_GROW_STUTTER,
    [SPECIES_LATIAS]     = BACK_ANIM_H_VIBRATE,
    [SPECIES_LATIOS]     = BACK_ANIM_H_VIBRATE,
    [SPECIES_JIRACHI]    = BACK_ANIM_CONVEX_DOUBLE_ARC,
    [SPECIES_DEOXYS]     = BACK_ANIM_SHRINK_GROW_VIBRATE,
    [SPECIES_CHIMECHO]   = BACK_ANIM_CONVEX_DOUBLE_ARC,
};

// Equivalent to struct YellowFlashData, but doesn't match as a struct
static const u8 sYellowFlashData[][2] =
{
    {FALSE,  5},
    { TRUE,  1},
    {FALSE, 15},
    { TRUE,  4},
    {FALSE,  2},
    { TRUE,  2},
    {FALSE,  2},
    { TRUE,  2},
    {FALSE,  2},
    { TRUE,  2},
    {FALSE,  2},
    { TRUE,  2},
    {FALSE,  2},
    {FALSE, -1}
};

static const u8 sVerticalShakeData[][2] =
{
    { 6,  30},
    {-2,  15},
    { 6,  30},
    {-1,   0}
};

static void (* const sMonAnimFunctions[])(struct Sprite *sprite) =
{
    [ANIM_V_SQUISH_AND_BOUNCE]               = Anim_VerticalSquishBounce,
    [ANIM_CIRCULAR_STRETCH_TWICE]            = Anim_CircularStretchTwice,
    [ANIM_H_VIBRATE]                         = Anim_HorizontalVibrate,
    [ANIM_H_SLIDE]                           = Anim_HorizontalSlide,
    [ANIM_V_SLIDE]                           = Anim_VerticalSlide,
    [ANIM_BOUNCE_ROTATE_TO_SIDES]            = Anim_BounceRotateToSides,
    [ANIM_V_JUMPS_H_JUMPS]                   = Anim_VerticalJumpsHorizontalJumps,
    [ANIM_ROTATE_TO_SIDES]                   = Anim_RotateToSides, // Unused
    [ANIM_ROTATE_TO_SIDES_TWICE]             = Anim_RotateToSides_Twice,
    [ANIM_GROW_VIBRATE]                      = Anim_GrowVibrate,
    [ANIM_ZIGZAG_FAST]                       = Anim_ZigzagFast,
    [ANIM_SWING_CONCAVE]                     = Anim_SwingConcave,
    [ANIM_SWING_CONCAVE_FAST]                = Anim_SwingConcave_Fast,
    [ANIM_SWING_CONVEX]                      = Anim_SwingConvex,
    [ANIM_SWING_CONVEX_FAST]                 = Anim_SwingConvex_Fast,
    [ANIM_H_SHAKE]                           = Anim_HorizontalShake,
    [ANIM_V_SHAKE]                           = Anim_VerticalShake,
    [ANIM_CIRCULAR_VIBRATE]                  = Anim_CircularVibrate,
    [ANIM_TWIST]                             = Anim_Twist,
    [ANIM_SHRINK_GROW]                       = Anim_ShrinkGrow,
    [ANIM_CIRCLE_C_CLOCKWISE]                = Anim_CircleCounterclockwise,
    [ANIM_GLOW_BLACK]                        = Anim_GlowBlack,
    [ANIM_H_STRETCH]                         = Anim_HorizontalStretch,
    [ANIM_V_STRETCH]                         = Anim_VerticalStretch,
    [ANIM_RISING_WOBBLE]                     = Anim_RisingWobble,
    [ANIM_V_SHAKE_TWICE]                     = Anim_VerticalShakeTwice,
    [ANIM_TIP_MOVE_FORWARD]                  = Anim_TipMoveForward,
    [ANIM_H_PIVOT]                           = Anim_HorizontalPivot,
    [ANIM_V_SLIDE_WOBBLE]                    = Anim_VerticalSlideWobble,
    [ANIM_H_SLIDE_WOBBLE]                    = Anim_HorizontalSlideWobble,
    [ANIM_V_JUMPS_BIG]                       = Anim_VerticalJumps_Big,
    [ANIM_SPIN_LONG]                         = Anim_Spin_Long, // Unused
    [ANIM_GLOW_ORANGE]                       = Anim_GlowOrange,
    [ANIM_GLOW_RED]                          = Anim_GlowRed, // Unused
    [ANIM_GLOW_BLUE]                         = Anim_GlowBlue,
    [ANIM_GLOW_YELLOW]                       = Anim_GlowYellow, // Unused
    [ANIM_GLOW_PURPLE]                       = Anim_GlowPurple, // Unused
    [ANIM_BACK_AND_LUNGE]                    = Anim_BackAndLunge,
    [ANIM_BACK_FLIP]                         = Anim_BackFlip, // Unused
    [ANIM_FLICKER]                           = Anim_Flicker, // Unused
    [ANIM_BACK_FLIP_BIG]                     = Anim_BackFlipBig, // Unused
    [ANIM_FRONT_FLIP]                        = Anim_FrontFlip,
    [ANIM_TUMBLING_FRONT_FLIP]               = Anim_TumblingFrontFlip, // Unused
    [ANIM_FIGURE_8]                          = Anim_Figure8,
    [ANIM_FLASH_YELLOW]                      = Anim_FlashYellow,
    [ANIM_SWING_CONCAVE_FAST_SHORT]          = Anim_SwingConcave_FastShort,
    [ANIM_SWING_CONVEX_FAST_SHORT]           = Anim_SwingConvex_FastShort, // Unused
    [ANIM_ROTATE_UP_SLAM_DOWN]               = Anim_RotateUpSlamDown,
    [ANIM_DEEP_V_SQUISH_AND_BOUNCE]          = Anim_DeepVerticalSquishBounce,
    [ANIM_H_JUMPS]                           = Anim_HorizontalJumps,
    [ANIM_H_JUMPS_V_STRETCH]                 = Anim_HorizontalJumpsVerticalStretch,
    [ANIM_ROTATE_TO_SIDES_FAST]              = Anim_RotateToSides_Fast, // Unused
    [ANIM_ROTATE_UP_TO_SIDES]                = Anim_RotateUpToSides,
    [ANIM_FLICKER_INCREASING]                = Anim_FlickerIncreasing,
    [ANIM_TIP_HOP_FORWARD]                   = Anim_TipHopForward, // Unused
    [ANIM_PIVOT_SHAKE]                       = Anim_PivotShake, // Unused
    [ANIM_TIP_AND_SHAKE]                     = Anim_TipAndShake, // Unused
    [ANIM_VIBRATE_TO_CORNERS]                = Anim_VibrateToCorners, // Unused
    [ANIM_GROW_IN_STAGES]                    = Anim_GrowInStages,
    [ANIM_V_SPRING]                          = Anim_VerticalSpring, // Unused
    [ANIM_V_REPEATED_SPRING]                 = Anim_VerticalRepeatedSpring, // Unused
    [ANIM_SPRING_RISING]                     = Anim_SpringRising, // Unused
    [ANIM_H_SPRING]                          = Anim_HorizontalSpring,
    [ANIM_H_REPEATED_SPRING_SLOW]            = Anim_HorizontalRepeatedSpring_Slow,
    [ANIM_H_SLIDE_SHRINK]                    = Anim_HorizontalSlideShrink, // Unused
    [ANIM_LUNGE_GROW]                        = Anim_LungeGrow,
    [ANIM_CIRCLE_INTO_BG]                    = Anim_CircleIntoBackground,
    [ANIM_RAPID_H_HOPS]                      = Anim_RapidHorizontalHops,
    [ANIM_FOUR_PETAL]                        = Anim_FourPetal,
    [ANIM_V_SQUISH_AND_BOUNCE_SLOW]          = Anim_VerticalSquishBounce_Slow,
    [ANIM_H_SLIDE_SLOW]                      = Anim_HorizontalSlide_Slow,
    [ANIM_V_SLIDE_SLOW]                      = Anim_VerticalSlide_Slow,
    [ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL]      = Anim_BounceRotateToSides_Small,
    [ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW]       = Anim_BounceRotateToSides_Slow,
    [ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL_SLOW] = Anim_BounceRotateToSides_SmallSlow,
    [ANIM_ZIGZAG_SLOW]                       = Anim_ZigzagSlow,
    [ANIM_H_SHAKE_SLOW]                      = Anim_HorizontalShake_Slow,
    [ANIM_V_SHAKE_SLOW]                      = Anim_VertialShake_Slow, // Unused
    [ANIM_TWIST_TWICE]                       = Anim_Twist_Twice,
    [ANIM_CIRCLE_C_CLOCKWISE_SLOW]           = Anim_CircleCounterclockwise_Slow,
    [ANIM_V_SHAKE_TWICE_SLOW]                = Anim_VerticalShakeTwice_Slow, // Unused
    [ANIM_V_SLIDE_WOBBLE_SMALL]              = Anim_VerticalSlideWobble_Small,
    [ANIM_V_JUMPS_SMALL]                     = Anim_VerticalJumps_Small,
    [ANIM_SPIN]                              = Anim_Spin,
    [ANIM_TUMBLING_FRONT_FLIP_TWICE]         = Anim_TumblingFrontFlip_Twice,
    [ANIM_DEEP_V_SQUISH_AND_BOUNCE_TWICE]    = Anim_DeepVerticalSquishBounce_Twice, // Unused
    [ANIM_H_JUMPS_V_STRETCH_TWICE]           = Anim_HorizontalJumpsVerticalStretch_Twice,
    [ANIM_V_SHAKE_BACK]                      = Anim_VerticalShakeBack,
    [ANIM_V_SHAKE_BACK_SLOW]                 = Anim_VerticalShakeBack_Slow,
    [ANIM_V_SHAKE_H_SLIDE_SLOW]              = Anim_VerticalShakeHorizontalSlide_Slow,
    [ANIM_V_STRETCH_BOTH_ENDS_SLOW]          = Anim_VerticalStretchBothEnds_Slow,
    [ANIM_H_STRETCH_FAR_SLOW]                = Anim_HorizontalStretchFar_Slow,
    [ANIM_V_SHAKE_LOW_TWICE]                 = Anim_VerticalShakeLowTwice,
    [ANIM_H_SHAKE_FAST]                      = Anim_HorizontalShake_Fast,
    [ANIM_H_SLIDE_FAST]                      = Anim_HorizontalSlide_Fast,
    [ANIM_H_VIBRATE_FAST]                    = Anim_HorizontalVibrate_Fast,
    [ANIM_H_VIBRATE_FASTEST]                 = Anim_HorizontalVibrate_Fastest,
    [ANIM_V_SHAKE_BACK_FAST]                 = Anim_VerticalShakeBack_Fast,
    [ANIM_V_SHAKE_LOW_TWICE_SLOW]            = Anim_VerticalShakeLowTwice_Slow,
    [ANIM_V_SHAKE_LOW_TWICE_FAST]            = Anim_VerticalShakeLowTwice_Fast,
    [ANIM_CIRCLE_C_CLOCKWISE_LONG]           = Anim_CircleCounterclockwise_Long,
    [ANIM_GROW_STUTTER_SLOW]                 = Anim_GrowStutter_Slow,
    [ANIM_V_SHAKE_H_SLIDE]                   = Anim_VerticalShakeHorizontalSlide,
    [ANIM_V_SHAKE_H_SLIDE_FAST]              = Anim_VerticalShakeHorizontalSlide_Fast,
    [ANIM_TRIANGLE_DOWN_SLOW]                = Anim_TriangleDown_Slow,
    [ANIM_TRIANGLE_DOWN]                     = Anim_TriangleDown,
    [ANIM_TRIANGLE_DOWN_TWICE]               = Anim_TriangleDown_Fast,
    [ANIM_GROW]                              = Anim_Grow,
    [ANIM_GROW_TWICE]                        = Anim_Grow_Twice,
    [ANIM_H_SPRING_FAST]                     = Anim_HorizontalSpring_Fast,
    [ANIM_H_SPRING_SLOW]                     = Anim_HorizontalSpring_Slow,
    [ANIM_H_REPEATED_SPRING_FAST]            = Anim_HorizontalRepeatedSpring_Fast,
    [ANIM_H_REPEATED_SPRING]                 = Anim_HorizontalRepeatedSpring,
    [ANIM_SHRINK_GROW_FAST]                  = Anim_ShrinkGrow_Fast,
    [ANIM_SHRINK_GROW_SLOW]                  = Anim_ShrinkGrow_Slow,
    [ANIM_V_STRETCH_BOTH_ENDS]               = Anim_VerticalStretchBothEnds,
    [ANIM_V_STRETCH_BOTH_ENDS_TWICE]         = Anim_VerticalStretchBothEnds_Twice,
    [ANIM_H_STRETCH_FAR_TWICE]               = Anim_HorizontalStretchFar_Twice,
    [ANIM_H_STRETCH_FAR]                     = Anim_HorizontalStretchFar,
    [ANIM_GROW_STUTTER_TWICE]                = Anim_GrowStutter_Twice,
    [ANIM_GROW_STUTTER]                      = Anim_GrowStutter,
    [ANIM_CONCAVE_ARC_LARGE_SLOW]            = Anim_ConcaveArcLarge_Slow,
    [ANIM_CONCAVE_ARC_LARGE]                 = Anim_ConcaveArcLarge,
    [ANIM_CONCAVE_ARC_LARGE_TWICE]           = Anim_ConcaveArcLarge_Twice,
    [ANIM_CONVEX_DOUBLE_ARC_SLOW]            = Anim_ConvexDoubleArc_Slow,
    [ANIM_CONVEX_DOUBLE_ARC]                 = Anim_ConvexDoubleArc,
    [ANIM_CONVEX_DOUBLE_ARC_TWICE]           = Anim_ConvexDoubleArc_Twice,
    [ANIM_CONCAVE_ARC_SMALL_SLOW]            = Anim_ConcaveArcSmall_Slow,
    [ANIM_CONCAVE_ARC_SMALL]                 = Anim_ConcaveArcSmall,
    [ANIM_CONCAVE_ARC_SMALL_TWICE]           = Anim_ConcaveArcSmall_Twice,
    [ANIM_H_DIP]                             = Anim_HorizontalDip,
    [ANIM_H_DIP_FAST]                        = Anim_HorizontalDip_Fast,
    [ANIM_H_DIP_TWICE]                       = Anim_HorizontalDip_Twice,
    [ANIM_SHRINK_GROW_VIBRATE_FAST]          = Anim_ShrinkGrowVibrate_Fast,
    [ANIM_SHRINK_GROW_VIBRATE]               = Anim_ShrinkGrowVibrate,
    [ANIM_SHRINK_GROW_VIBRATE_SLOW]          = Anim_ShrinkGrowVibrate_Slow,
    [ANIM_JOLT_RIGHT_FAST]                   = Anim_JoltRight_Fast,
    [ANIM_JOLT_RIGHT]                        = Anim_JoltRight,
    [ANIM_JOLT_RIGHT_SLOW]                   = Anim_JoltRight_Slow,
    [ANIM_SHAKE_FLASH_YELLOW_FAST]           = Anim_ShakeFlashYellow_Fast,
    [ANIM_SHAKE_FLASH_YELLOW]                = Anim_ShakeFlashYellow,
    [ANIM_SHAKE_FLASH_YELLOW_SLOW]           = Anim_ShakeFlashYellow_Slow,
    [ANIM_SHAKE_GLOW_RED_FAST]               = Anim_ShakeGlowRed_Fast,
    [ANIM_SHAKE_GLOW_RED]                    = Anim_ShakeGlowRed,
    [ANIM_SHAKE_GLOW_RED_SLOW]               = Anim_ShakeGlowRed_Slow,
    [ANIM_SHAKE_GLOW_GREEN_FAST]             = Anim_ShakeGlowGreen_Fast,
    [ANIM_SHAKE_GLOW_GREEN]                  = Anim_ShakeGlowGreen,
    [ANIM_SHAKE_GLOW_GREEN_SLOW]             = Anim_ShakeGlowGreen_Slow,
    [ANIM_SHAKE_GLOW_BLUE_FAST]              = Anim_ShakeGlowBlue_Fast,
    [ANIM_SHAKE_GLOW_BLUE]                   = Anim_ShakeGlowBlue,
    [ANIM_SHAKE_GLOW_BLUE_SLOW]              = Anim_ShakeGlowBlue_Slow,
    [ANIM_SHAKE_GLOW_BLACK_SLOW]             = Anim_ShakeGlowBlack_Slow,
    [ANIM_SHAKE_GLOW_WHITE_SLOW]             = Anim_ShakeGlowWhite_Slow,
    [ANIM_SHAKE_GLOW_PURPLE_SLOW]            = Anim_ShakeGlowPurple_Slow
};

// Each back anim set has 3 possible animations depending on nature
// Each of the 3 animations is a slight variation of the others
// BACK_ANIM_NONE is skipped below. GetSpeciesBackAnimSet subtracts 1 from the back anim id
static const u8 sBackAnimationIds[] =
{
    [(BACK_ANIM_H_VIBRATE - 1) * 3]               = ANIM_H_VIBRATE_FASTEST, ANIM_H_VIBRATE_FAST, ANIM_H_VIBRATE,
    [(BACK_ANIM_H_SLIDE - 1) * 3]                 = ANIM_H_SLIDE_FAST, ANIM_H_SLIDE, ANIM_H_SLIDE_SLOW,
    [(BACK_ANIM_H_SPRING - 1) * 3]                = ANIM_H_SPRING_FAST, ANIM_H_SPRING, ANIM_H_SPRING_SLOW,
    [(BACK_ANIM_H_SPRING_REPEATED - 1) * 3]       = ANIM_H_REPEATED_SPRING_FAST, ANIM_H_REPEATED_SPRING, ANIM_H_REPEATED_SPRING_SLOW,
    [(BACK_ANIM_SHRINK_GROW - 1) * 3]             = ANIM_SHRINK_GROW_FAST, ANIM_SHRINK_GROW, ANIM_SHRINK_GROW_SLOW,
    [(BACK_ANIM_GROW - 1) * 3]                    = ANIM_GROW_TWICE, ANIM_GROW, ANIM_GROW_IN_STAGES,
    [(BACK_ANIM_CIRCLE_COUNTERCLOCKWISE - 1) * 3] = ANIM_CIRCLE_C_CLOCKWISE_LONG, ANIM_CIRCLE_C_CLOCKWISE, ANIM_CIRCLE_C_CLOCKWISE_SLOW,
    [(BACK_ANIM_H_SHAKE - 1) * 3]                 = ANIM_H_SHAKE_FAST, ANIM_H_SHAKE, ANIM_H_SHAKE_SLOW,
    [(BACK_ANIM_V_SHAKE - 1) * 3]                 = ANIM_V_SHAKE_BACK_FAST, ANIM_V_SHAKE_BACK, ANIM_V_SHAKE_BACK_SLOW,
    [(BACK_ANIM_V_SHAKE_H_SLIDE - 1) * 3]         = ANIM_V_SHAKE_H_SLIDE_FAST, ANIM_V_SHAKE_H_SLIDE, ANIM_V_SHAKE_H_SLIDE_SLOW,
    [(BACK_ANIM_V_STRETCH - 1) * 3]               = ANIM_V_STRETCH_BOTH_ENDS_TWICE, ANIM_V_STRETCH_BOTH_ENDS, ANIM_V_STRETCH_BOTH_ENDS_SLOW,
    [(BACK_ANIM_H_STRETCH - 1) * 3]               = ANIM_H_STRETCH_FAR_TWICE, ANIM_H_STRETCH_FAR, ANIM_H_STRETCH_FAR_SLOW,
    [(BACK_ANIM_GROW_STUTTER - 1) * 3]            = ANIM_GROW_STUTTER_TWICE, ANIM_GROW_STUTTER, ANIM_GROW_STUTTER_SLOW,
    [(BACK_ANIM_V_SHAKE_LOW - 1) * 3]             = ANIM_V_SHAKE_LOW_TWICE_FAST, ANIM_V_SHAKE_LOW_TWICE, ANIM_V_SHAKE_LOW_TWICE_SLOW,
    [(BACK_ANIM_TRIANGLE_DOWN - 1) * 3]           = ANIM_TRIANGLE_DOWN_TWICE, ANIM_TRIANGLE_DOWN, ANIM_TRIANGLE_DOWN_SLOW,
    [(BACK_ANIM_CONCAVE_ARC_LARGE - 1) * 3]       = ANIM_CONCAVE_ARC_LARGE_TWICE, ANIM_CONCAVE_ARC_LARGE, ANIM_CONCAVE_ARC_LARGE_SLOW,
    [(BACK_ANIM_CONVEX_DOUBLE_ARC - 1) * 3]       = ANIM_CONVEX_DOUBLE_ARC_TWICE, ANIM_CONVEX_DOUBLE_ARC, ANIM_CONVEX_DOUBLE_ARC_SLOW,
    [(BACK_ANIM_CONCAVE_ARC_SMALL - 1) * 3]       = ANIM_CONCAVE_ARC_SMALL_TWICE, ANIM_CONCAVE_ARC_SMALL, ANIM_CONCAVE_ARC_SMALL_SLOW,
    [(BACK_ANIM_DIP_RIGHT_SIDE - 1) * 3]          = ANIM_H_DIP_TWICE, ANIM_H_DIP, ANIM_H_DIP_FAST,
    [(BACK_ANIM_SHRINK_GROW_VIBRATE - 1) * 3]     = ANIM_SHRINK_GROW_VIBRATE_FAST, ANIM_SHRINK_GROW_VIBRATE, ANIM_SHRINK_GROW_VIBRATE_SLOW,
    [(BACK_ANIM_JOLT_RIGHT - 1) * 3]              = ANIM_JOLT_RIGHT_FAST, ANIM_JOLT_RIGHT, ANIM_JOLT_RIGHT_SLOW,
    [(BACK_ANIM_SHAKE_FLASH_YELLOW - 1) * 3]      = ANIM_SHAKE_FLASH_YELLOW_FAST, ANIM_SHAKE_FLASH_YELLOW, ANIM_SHAKE_FLASH_YELLOW_SLOW,
    [(BACK_ANIM_SHAKE_GLOW_RED - 1) * 3]          = ANIM_SHAKE_GLOW_RED_FAST, ANIM_SHAKE_GLOW_RED, ANIM_SHAKE_GLOW_RED_SLOW,
    [(BACK_ANIM_SHAKE_GLOW_GREEN - 1) * 3]        = ANIM_SHAKE_GLOW_GREEN_FAST, ANIM_SHAKE_GLOW_GREEN, ANIM_SHAKE_GLOW_GREEN_SLOW,
    [(BACK_ANIM_SHAKE_GLOW_BLUE - 1) * 3]         = ANIM_SHAKE_GLOW_BLUE_FAST,  ANIM_SHAKE_GLOW_BLUE, ANIM_SHAKE_GLOW_BLUE_SLOW,
};

static const u8 sBackAnimNatureModTable[NUM_NATURES] =
{
    [NATURE_HARDY]   = 0,
    [NATURE_LONELY]  = 2,
    [NATURE_BRAVE]   = 0,
    [NATURE_ADAMANT] = 0,
    [NATURE_NAUGHTY] = 0,
    [NATURE_BOLD]    = 1,
    [NATURE_DOCILE]  = 1,
    [NATURE_RELAXED] = 1,
    [NATURE_IMPISH]  = 0,
    [NATURE_LAX]     = 1,
    [NATURE_TIMID]   = 2,
    [NATURE_HASTY]   = 0,
    [NATURE_SERIOUS] = 1,
    [NATURE_JOLLY]   = 0,
    [NATURE_NAIVE]   = 0,
    [NATURE_MODEST]  = 2,
    [NATURE_MILD]    = 2,
    [NATURE_QUIET]   = 2,
    [NATURE_BASHFUL] = 2,
    [NATURE_RASH]    = 1,
    [NATURE_CALM]    = 1,
    [NATURE_GENTLE]  = 2,
    [NATURE_SASSY]   = 1,
    [NATURE_CAREFUL] = 2,
    [NATURE_QUIRKY]  = 1,
};

// FRONT MON ANIMATIONS

// SPECIES_NONE are ignored in the following two tables, so decrement before accessing these arrays to get the right result
#if P_ENABLE_DEBUG == TRUE
const u8 sMonFrontAnimIdsTable[NUM_SPECIES - 1] =
#else
static const u8 sMonFrontAnimIdsTable[NUM_SPECIES - 1] =
#endif
{
    [SPECIES_BULBASAUR - 1]     = ANIM_V_JUMPS_H_JUMPS,
    [SPECIES_IVYSAUR - 1]       = ANIM_V_STRETCH,
    [SPECIES_VENUSAUR - 1]      = ANIM_ROTATE_UP_SLAM_DOWN,
    [SPECIES_CHARMANDER - 1]    = ANIM_V_JUMPS_SMALL,
    [SPECIES_CHARMELEON - 1]    = ANIM_BACK_AND_LUNGE,
    [SPECIES_CHARIZARD - 1]     = ANIM_V_SHAKE,
    [SPECIES_SQUIRTLE - 1]      = ANIM_SWING_CONCAVE,
    [SPECIES_WARTORTLE - 1]     = ANIM_SHRINK_GROW,
    [SPECIES_BLASTOISE - 1]     = ANIM_V_SHAKE_TWICE,
    [SPECIES_CATERPIE - 1]      = ANIM_SWING_CONCAVE,
    [SPECIES_METAPOD - 1]       = ANIM_SWING_CONCAVE,
    [SPECIES_BUTTERFREE - 1]    = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_WEEDLE - 1]        = ANIM_H_SLIDE_SLOW,
    [SPECIES_KAKUNA - 1]        = ANIM_GLOW_ORANGE,
    [SPECIES_BEEDRILL - 1]      = ANIM_H_VIBRATE,
    [SPECIES_PIDGEY - 1]        = ANIM_V_STRETCH,
    [SPECIES_PIDGEOTTO - 1]     = ANIM_V_STRETCH,
    [SPECIES_PIDGEOT - 1]       = ANIM_FRONT_FLIP,
    [SPECIES_RATTATA - 1]       = ANIM_RAPID_H_HOPS,
    [SPECIES_RATICATE - 1]      = ANIM_FIGURE_8,
    [SPECIES_SPEAROW - 1]       = ANIM_RISING_WOBBLE,
    [SPECIES_FEAROW - 1]        = ANIM_FIGURE_8,
    [SPECIES_EKANS - 1]         = ANIM_H_STRETCH,
    [SPECIES_ARBOK - 1]         = ANIM_V_STRETCH,
    [SPECIES_PIKACHU - 1]       = ANIM_FLASH_YELLOW,
    [SPECIES_RAICHU - 1]        = ANIM_V_STRETCH,
    [SPECIES_SANDSHREW - 1]     = ANIM_SWING_CONCAVE_FAST_SHORT,
    [SPECIES_SANDSLASH - 1]     = ANIM_V_STRETCH,
    [SPECIES_NIDORAN_F - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_NIDORINA - 1]      = ANIM_V_STRETCH,
    [SPECIES_NIDOQUEEN - 1]     = ANIM_H_SHAKE,
    [SPECIES_NIDORAN_M - 1]     = ANIM_GROW_VIBRATE,
    [SPECIES_NIDORINO - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_NIDOKING - 1]      = ANIM_H_SHAKE,
    [SPECIES_CLEFAIRY - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CLEFABLE - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL_SLOW,
    [SPECIES_VULPIX - 1]        = ANIM_V_STRETCH,
    [SPECIES_NINETALES - 1]     = ANIM_V_SHAKE,
    [SPECIES_JIGGLYPUFF - 1]    = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL,
    [SPECIES_WIGGLYTUFF - 1]    = ANIM_H_JUMPS,
    [SPECIES_ZUBAT - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GOLBAT - 1]        = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_ODDISH - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GLOOM - 1]         = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_VILEPLUME - 1]     = ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW,
    [SPECIES_PARAS - 1]         = ANIM_H_SLIDE_SLOW,
    [SPECIES_PARASECT - 1]      = ANIM_H_SHAKE,
    [SPECIES_VENONAT - 1]       = ANIM_V_JUMPS_H_JUMPS,
    [SPECIES_VENOMOTH - 1]      = ANIM_ZIGZAG_SLOW,
    [SPECIES_DIGLETT - 1]       = ANIM_V_SHAKE,
    [SPECIES_DUGTRIO - 1]       = ANIM_H_SHAKE_SLOW,
    [SPECIES_MEOWTH - 1]        = ANIM_V_JUMPS_SMALL,
    [SPECIES_PERSIAN - 1]       = ANIM_V_STRETCH,
    [SPECIES_PSYDUCK - 1]       = ANIM_V_JUMPS_H_JUMPS,
    [SPECIES_GOLDUCK - 1]       = ANIM_H_SHAKE_SLOW,
    [SPECIES_MANKEY - 1]        = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_PRIMEAPE - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL,
    [SPECIES_GROWLITHE - 1]     = ANIM_BACK_AND_LUNGE,
    [SPECIES_ARCANINE - 1]      = ANIM_H_VIBRATE,
    [SPECIES_POLIWAG - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_POLIWHIRL - 1]     = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_POLIWRATH - 1]     = ANIM_V_SHAKE_TWICE,
    [SPECIES_ABRA - 1]          = ANIM_H_JUMPS,
    [SPECIES_KADABRA - 1]       = ANIM_GROW_VIBRATE,
    [SPECIES_ALAKAZAM - 1]      = ANIM_V_STRETCH,
    [SPECIES_MACHOP - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MACHOKE - 1]       = ANIM_V_SHAKE,
    [SPECIES_MACHAMP - 1]       = ANIM_H_JUMPS,
    [SPECIES_BELLSPROUT - 1]    = ANIM_V_STRETCH,
    [SPECIES_WEEPINBELL - 1]    = ANIM_SWING_CONVEX,
    [SPECIES_VICTREEBEL - 1]    = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_TENTACOOL - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TENTACRUEL - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GEODUDE - 1]       = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL,
    [SPECIES_GRAVELER - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL,
    [SPECIES_GOLEM - 1]         = ANIM_ROTATE_UP_SLAM_DOWN,
    [SPECIES_PONYTA - 1]        = ANIM_GLOW_ORANGE,
    [SPECIES_RAPIDASH - 1]      = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_SLOWPOKE - 1]      = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_SLOWBRO - 1]       = ANIM_SWING_CONCAVE,
    [SPECIES_MAGNEMITE - 1]     = ANIM_TUMBLING_FRONT_FLIP_TWICE,
    [SPECIES_MAGNETON - 1]      = ANIM_FLASH_YELLOW,
    [SPECIES_FARFETCHD - 1]     = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL,
    [SPECIES_DODUO - 1]         = ANIM_H_SHAKE_SLOW,
    [SPECIES_DODRIO - 1]        = ANIM_LUNGE_GROW,
    [SPECIES_SEEL - 1]          = ANIM_SWING_CONCAVE,
    [SPECIES_DEWGONG - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_GRIMER - 1]        = ANIM_H_SLIDE_SLOW,
    [SPECIES_MUK - 1]           = ANIM_DEEP_V_SQUISH_AND_BOUNCE,
    [SPECIES_SHELLDER - 1]      = ANIM_TWIST,
    [SPECIES_CLOYSTER - 1]      = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_GASTLY - 1]        = ANIM_GLOW_BLACK,
    [SPECIES_HAUNTER - 1]       = ANIM_FLICKER_INCREASING,
    [SPECIES_GENGAR - 1]        = ANIM_GROW_IN_STAGES,
    [SPECIES_ONIX - 1]          = ANIM_RAPID_H_HOPS,
    [SPECIES_DROWZEE - 1]       = ANIM_CIRCLE_C_CLOCKWISE_SLOW,
    [SPECIES_HYPNO - 1]         = ANIM_GROW_VIBRATE,
    [SPECIES_KRABBY - 1]        = ANIM_H_SLIDE,
    [SPECIES_KINGLER - 1]       = ANIM_ZIGZAG_SLOW,
    [SPECIES_VOLTORB - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ELECTRODE - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_EXEGGCUTE - 1]     = ANIM_H_SLIDE_SLOW,
    [SPECIES_EXEGGUTOR - 1]     = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_CUBONE - 1]        = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL,
    [SPECIES_MAROWAK - 1]       = ANIM_BOUNCE_ROTATE_TO_SIDES,
    [SPECIES_HITMONLEE - 1]     = ANIM_H_STRETCH,
    [SPECIES_HITMONCHAN - 1]    = ANIM_GROW_VIBRATE,
    [SPECIES_LICKITUNG - 1]     = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_KOFFING - 1]       = ANIM_SHRINK_GROW,
    [SPECIES_WEEZING - 1]       = ANIM_V_SLIDE,
    [SPECIES_RHYHORN - 1]       = ANIM_V_SHAKE,
    [SPECIES_RHYDON - 1]        = ANIM_SHRINK_GROW,
    [SPECIES_CHANSEY - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_TANGELA - 1]       = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL,
    [SPECIES_KANGASKHAN - 1]    = ANIM_V_STRETCH,
    [SPECIES_HORSEA - 1]        = ANIM_TWIST,
    [SPECIES_SEADRA - 1]        = ANIM_V_SLIDE,
    [SPECIES_GOLDEEN - 1]       = ANIM_SWING_CONVEX,
    [SPECIES_SEAKING - 1]       = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_STARYU - 1]        = ANIM_TWIST_TWICE,
    [SPECIES_STARMIE - 1]       = ANIM_TWIST,
    [SPECIES_MR_MIME - 1]       = ANIM_H_SLIDE_SLOW,
    [SPECIES_SCYTHER - 1]       = ANIM_H_VIBRATE,
    [SPECIES_JYNX - 1]          = ANIM_V_STRETCH,
    [SPECIES_ELECTABUZZ - 1]    = ANIM_FLASH_YELLOW,
    [SPECIES_MAGMAR - 1]        = ANIM_H_SHAKE,
    [SPECIES_PINSIR - 1]        = ANIM_GROW_VIBRATE,
    [SPECIES_TAUROS - 1]        = ANIM_V_SHAKE_TWICE,
    [SPECIES_MAGIKARP - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES,
    [SPECIES_GYARADOS - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL,
    [SPECIES_LAPRAS - 1]        = ANIM_V_STRETCH,
    [SPECIES_DITTO - 1]         = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_EEVEE - 1]         = ANIM_V_STRETCH,
    [SPECIES_VAPOREON - 1]      = ANIM_V_STRETCH,
    [SPECIES_JOLTEON - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FLAREON - 1]       = ANIM_V_STRETCH,
    [SPECIES_PORYGON - 1]       = ANIM_V_JUMPS_SMALL,
    [SPECIES_OMANYTE - 1]       = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_OMASTAR - 1]       = ANIM_GROW_VIBRATE,
    [SPECIES_KABUTO - 1]        = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_KABUTOPS - 1]      = ANIM_H_SHAKE,
    [SPECIES_AERODACTYL - 1]    = ANIM_V_SLIDE_SLOW,
    [SPECIES_SNORLAX - 1]       = ANIM_SWING_CONCAVE,
    [SPECIES_ARTICUNO - 1]      = ANIM_GROW_VIBRATE,
    [SPECIES_ZAPDOS - 1]        = ANIM_FLASH_YELLOW,
    [SPECIES_MOLTRES - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_DRATINI - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DRAGONAIR - 1]     = ANIM_V_SHAKE,
    [SPECIES_DRAGONITE - 1]     = ANIM_V_SLIDE_SLOW,
    [SPECIES_MEWTWO - 1]        = ANIM_GROW_VIBRATE,
    [SPECIES_MEW - 1]           = ANIM_SWING_CONVEX,
    [SPECIES_CHIKORITA - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BAYLEEF - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MEGANIUM - 1]      = ANIM_V_STRETCH,
    [SPECIES_CYNDAQUIL - 1]     = ANIM_V_JUMPS_SMALL,
    [SPECIES_QUILAVA - 1]       = ANIM_V_STRETCH,
    [SPECIES_TYPHLOSION - 1]    = ANIM_V_SHAKE,
    [SPECIES_TOTODILE - 1]      = ANIM_H_JUMPS,
    [SPECIES_CROCONAW - 1]      = ANIM_H_SHAKE,
    [SPECIES_FERALIGATR - 1]    = ANIM_H_SHAKE,
    [SPECIES_SENTRET - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FURRET - 1]        = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_HOOTHOOT - 1]      = ANIM_V_SLIDE_SLOW,
    [SPECIES_NOCTOWL - 1]       = ANIM_V_STRETCH,
    [SPECIES_LEDYBA - 1]        = ANIM_V_JUMPS_SMALL,
    [SPECIES_LEDIAN - 1]        = ANIM_V_SLIDE_SLOW,
    [SPECIES_SPINARAK - 1]      = ANIM_CIRCLE_C_CLOCKWISE_SLOW,
    [SPECIES_ARIADOS - 1]       = ANIM_H_SHAKE,
    [SPECIES_CROBAT - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CHINCHOU - 1]      = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_LANTURN - 1]       = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_PICHU - 1]         = ANIM_V_JUMPS_BIG,
    [SPECIES_CLEFFA - 1]        = ANIM_V_JUMPS_SMALL,
    [SPECIES_IGGLYBUFF - 1]     = ANIM_SWING_CONCAVE_FAST,
    [SPECIES_TOGEPI - 1]        = ANIM_SWING_CONCAVE,
    [SPECIES_TOGETIC - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_NATU - 1]          = ANIM_H_JUMPS,
    [SPECIES_XATU - 1]          = ANIM_GROW_VIBRATE,
    [SPECIES_MAREEP - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FLAAFFY - 1]       = ANIM_V_JUMPS_BIG,
    [SPECIES_AMPHAROS - 1]      = ANIM_FLASH_YELLOW,
    [SPECIES_BELLOSSOM - 1]     = ANIM_SWING_CONCAVE,
    [SPECIES_MARILL - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_AZUMARILL - 1]     = ANIM_BOUNCE_ROTATE_TO_SIDES_SMALL_SLOW,
    [SPECIES_SUDOWOODO - 1]     = ANIM_H_SLIDE_SLOW,
    [SPECIES_POLITOED - 1]      = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_HOPPIP - 1]        = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_SKIPLOOM - 1]      = ANIM_RISING_WOBBLE,
    [SPECIES_JUMPLUFF - 1]      = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_AIPOM - 1]         = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_SUNKERN - 1]       = ANIM_V_JUMPS_SMALL,
    [SPECIES_SUNFLORA - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_YANMA - 1]         = ANIM_FIGURE_8,
    [SPECIES_WOOPER - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_QUAGSIRE - 1]      = ANIM_H_STRETCH,
    [SPECIES_ESPEON - 1]        = ANIM_GROW_VIBRATE,
    [SPECIES_UMBREON - 1]       = ANIM_V_SHAKE,
    [SPECIES_MURKROW - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SLOWKING - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_MISDREAVUS - 1]    = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_UNOWN - 1]         = ANIM_ZIGZAG_FAST,
    [SPECIES_WOBBUFFET - 1]     = ANIM_DEEP_V_SQUISH_AND_BOUNCE,
    [SPECIES_GIRAFARIG - 1]     = ANIM_V_JUMPS_BIG,
    [SPECIES_PINECO - 1]        = ANIM_SWING_CONCAVE,
    [SPECIES_FORRETRESS - 1]    = ANIM_V_SHAKE,
    [SPECIES_DUNSPARCE - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GLIGAR - 1]        = ANIM_SHRINK_GROW,
    [SPECIES_STEELIX - 1]       = ANIM_H_SHAKE,
    [SPECIES_SNUBBULL - 1]      = ANIM_V_STRETCH,
    [SPECIES_GRANBULL - 1]      = ANIM_V_SHAKE,
    [SPECIES_QWILFISH - 1]      = ANIM_GROW_IN_STAGES,
    [SPECIES_SCIZOR - 1]        = ANIM_H_VIBRATE,
    [SPECIES_SHUCKLE - 1]       = ANIM_SWING_CONCAVE,
    [SPECIES_HERACROSS - 1]     = ANIM_LUNGE_GROW,
    [SPECIES_SNEASEL - 1]       = ANIM_H_STRETCH,
    [SPECIES_TEDDIURSA - 1]     = ANIM_V_STRETCH,
    [SPECIES_URSARING - 1]      = ANIM_V_SHAKE,
    [SPECIES_SLUGMA - 1]        = ANIM_V_STRETCH,
    [SPECIES_MAGCARGO - 1]      = ANIM_V_STRETCH,
    [SPECIES_SWINUB - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PILOSWINE - 1]     = ANIM_H_SHAKE,
    [SPECIES_CORSOLA - 1]       = ANIM_H_SLIDE,
    [SPECIES_REMORAID - 1]      = ANIM_V_JUMPS_SMALL,
    [SPECIES_OCTILLERY - 1]     = ANIM_V_STRETCH,
    [SPECIES_DELIBIRD - 1]      = ANIM_V_JUMPS_SMALL,
    [SPECIES_MANTINE - 1]       = ANIM_SWING_CONVEX,
    [SPECIES_SKARMORY - 1]      = ANIM_V_STRETCH,
    [SPECIES_HOUNDOUR - 1]      = ANIM_V_STRETCH,
    [SPECIES_HOUNDOOM - 1]      = ANIM_V_SHAKE,
    [SPECIES_KINGDRA - 1]       = ANIM_CIRCLE_INTO_BG,
    [SPECIES_PHANPY - 1]        = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_DONPHAN - 1]       = ANIM_V_SHAKE_TWICE,
    [SPECIES_PORYGON2 - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_STANTLER - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SMEARGLE - 1]      = ANIM_H_JUMPS,
    [SPECIES_TYROGUE - 1]       = ANIM_H_STRETCH,
    [SPECIES_HITMONTOP - 1]     = ANIM_H_VIBRATE,
    [SPECIES_SMOOCHUM - 1]      = ANIM_GROW_VIBRATE,
    [SPECIES_ELEKID - 1]        = ANIM_FLASH_YELLOW,
    [SPECIES_MAGBY - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MILTANK - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_BLISSEY - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_RAIKOU - 1]        = ANIM_FLASH_YELLOW,
    [SPECIES_ENTEI - 1]         = ANIM_GROW_VIBRATE,
    [SPECIES_SUICUNE - 1]       = ANIM_V_SHAKE,
    [SPECIES_LARVITAR - 1]      = ANIM_V_JUMPS_SMALL,
    [SPECIES_PUPITAR - 1]       = ANIM_V_SHAKE,
    [SPECIES_TYRANITAR - 1]     = ANIM_H_SHAKE,
    [SPECIES_LUGIA - 1]         = ANIM_GROW_IN_STAGES,
    [SPECIES_HO_OH - 1]         = ANIM_GROW_VIBRATE,
    [SPECIES_CELEBI - 1]        = ANIM_RISING_WOBBLE,
    [SPECIES_TREECKO - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GROVYLE - 1]       = ANIM_V_STRETCH,
    [SPECIES_SCEPTILE - 1]      = ANIM_V_SHAKE,
    [SPECIES_TORCHIC - 1]       = ANIM_H_STRETCH,
    [SPECIES_COMBUSKEN - 1]     = ANIM_V_JUMPS_H_JUMPS,
    [SPECIES_BLAZIKEN - 1]      = ANIM_H_SHAKE,
    [SPECIES_MUDKIP - 1]        = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_MARSHTOMP - 1]     = ANIM_V_SLIDE,
    [SPECIES_SWAMPERT - 1]      = ANIM_V_JUMPS_BIG,
    [SPECIES_POOCHYENA - 1]     = ANIM_V_SHAKE,
    [SPECIES_MIGHTYENA - 1]     = ANIM_V_SHAKE,
    [SPECIES_ZIGZAGOON - 1]     = ANIM_H_SLIDE,
    [SPECIES_LINOONE - 1]       = ANIM_GROW_VIBRATE,
    [SPECIES_WURMPLE - 1]       = ANIM_V_STRETCH,
    [SPECIES_SILCOON - 1]       = ANIM_V_SLIDE,
    [SPECIES_BEAUTIFLY - 1]     = ANIM_V_SLIDE,
    [SPECIES_CASCOON - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DUSTOX - 1]        = ANIM_V_JUMPS_H_JUMPS,
    [SPECIES_LOTAD - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LOMBRE - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LUDICOLO - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW,
    [SPECIES_SEEDOT - 1]        = ANIM_BOUNCE_ROTATE_TO_SIDES,
    [SPECIES_NUZLEAF - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SHIFTRY - 1]       = ANIM_H_VIBRATE,
    [SPECIES_TAILLOW - 1]       = ANIM_SWING_CONCAVE_FAST_SHORT,
    [SPECIES_SWELLOW - 1]       = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_WINGULL - 1]       = ANIM_V_JUMPS_BIG,
    [SPECIES_PELIPPER - 1]      = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_RALTS - 1]         = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_KIRLIA - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GARDEVOIR - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SURSKIT - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MASQUERAIN - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SHROOMISH - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BRELOOM - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SLAKOTH - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_VIGOROTH - 1]      = ANIM_H_JUMPS,
    [SPECIES_SLAKING - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_NINCADA - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_NINJASK - 1]       = ANIM_H_SLIDE_SLOW,
    [SPECIES_SHEDINJA - 1]      = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_WHISMUR - 1]       = ANIM_H_SLIDE,
    [SPECIES_LOUDRED - 1]       = ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW,
    [SPECIES_EXPLOUD - 1]       = ANIM_V_SHAKE_TWICE,
    [SPECIES_MAKUHITA - 1]      = ANIM_SWING_CONCAVE,
    [SPECIES_HARIYAMA - 1]      = ANIM_ROTATE_UP_TO_SIDES,
    [SPECIES_AZURILL - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_NOSEPASS - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW,
    [SPECIES_SKITTY - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DELCATTY - 1]      = ANIM_V_STRETCH,
    [SPECIES_SABLEYE - 1]       = ANIM_GLOW_BLACK,
    [SPECIES_MAWILE - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ARON - 1]          = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_LAIRON - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_AGGRON - 1]        = ANIM_V_SHAKE_TWICE,
    [SPECIES_MEDITITE - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES,
    [SPECIES_MEDICHAM - 1]      = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_ELECTRIKE - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MANECTRIC - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PLUSLE - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MINUN - 1]         = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_VOLBEAT - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ILLUMISE - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES,
    [SPECIES_ROSELIA - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_GULPIN - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SWALOT - 1]        = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_CARVANHA - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW,
    [SPECIES_SHARPEDO - 1]      = ANIM_H_JUMPS_V_STRETCH_TWICE,
    [SPECIES_WAILMER - 1]       = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_WAILORD - 1]       = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_NUMEL - 1]         = ANIM_V_SLIDE,
    [SPECIES_CAMERUPT - 1]      = ANIM_V_SHAKE,
    [SPECIES_TORKOAL - 1]       = ANIM_V_STRETCH,
    [SPECIES_SPOINK - 1]        = ANIM_H_JUMPS_V_STRETCH_TWICE,
    [SPECIES_GRUMPIG - 1]       = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_SPINDA - 1]        = ANIM_H_JUMPS,
    [SPECIES_TRAPINCH - 1]      = ANIM_V_SHAKE,
    [SPECIES_VIBRAVA - 1]       = ANIM_H_SHAKE,
    [SPECIES_FLYGON - 1]        = ANIM_ZIGZAG_SLOW,
    [SPECIES_CACNEA - 1]        = ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW,
    [SPECIES_CACTURNE - 1]      = ANIM_V_SLIDE,
    [SPECIES_SWABLU - 1]        = ANIM_V_SLIDE,
    [SPECIES_ALTARIA - 1]       = ANIM_H_STRETCH,
    [SPECIES_ZANGOOSE - 1]      = ANIM_GROW_VIBRATE,
    [SPECIES_SEVIPER - 1]       = ANIM_V_STRETCH,
    [SPECIES_LUNATONE - 1]      = ANIM_SWING_CONVEX_FAST,
    [SPECIES_SOLROCK - 1]       = ANIM_ROTATE_TO_SIDES_TWICE,
    [SPECIES_BARBOACH - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW,
    [SPECIES_WHISCASH - 1]      = ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW,
    [SPECIES_CORPHISH - 1]      = ANIM_V_SHAKE,
    [SPECIES_CRAWDAUNT - 1]     = ANIM_GROW_VIBRATE,
    [SPECIES_BALTOY - 1]        = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_CLAYDOL - 1]       = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_LILEEP - 1]        = ANIM_V_STRETCH,
    [SPECIES_CRADILY - 1]       = ANIM_V_SHAKE_TWICE,
    [SPECIES_ANORITH - 1]       = ANIM_TWIST,
    [SPECIES_ARMALDO - 1]       = ANIM_V_SHAKE,
    [SPECIES_FEEBAS - 1]        = ANIM_BOUNCE_ROTATE_TO_SIDES_SLOW,
    [SPECIES_MILOTIC - 1]       = ANIM_GLOW_BLUE,
    [SPECIES_CASTFORM - 1]      = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_KECLEON - 1]       = ANIM_FLICKER_INCREASING,
    [SPECIES_SHUPPET - 1]       = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_BANETTE - 1]       = ANIM_SWING_CONVEX,
    [SPECIES_DUSKULL - 1]       = ANIM_ZIGZAG_FAST,
    [SPECIES_DUSCLOPS - 1]      = ANIM_H_VIBRATE,
    [SPECIES_TROPIUS - 1]       = ANIM_V_SHAKE,
    [SPECIES_CHIMECHO - 1]      = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_ABSOL - 1]         = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_WYNAUT - 1]        = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_SNORUNT - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_GLALIE - 1]        = ANIM_ZIGZAG_FAST,
    [SPECIES_SPHEAL - 1]        = ANIM_SPIN,
    [SPECIES_SEALEO - 1]        = ANIM_V_STRETCH,
    [SPECIES_WALREIN - 1]       = ANIM_H_SHAKE,
    [SPECIES_CLAMPERL - 1]      = ANIM_TWIST,
    [SPECIES_HUNTAIL - 1]       = ANIM_GROW_VIBRATE,
    [SPECIES_GOREBYSS - 1]      = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_RELICANTH - 1]     = ANIM_TIP_MOVE_FORWARD,
    [SPECIES_LUVDISC - 1]       = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_BAGON - 1]         = ANIM_V_SHAKE_TWICE,
    [SPECIES_SHELGON - 1]       = ANIM_V_SLIDE,
    [SPECIES_SALAMENCE - 1]     = ANIM_H_SHAKE,
    [SPECIES_BELDUM - 1]        = ANIM_H_SHAKE,
    [SPECIES_METANG - 1]        = ANIM_V_SLIDE,
    [SPECIES_METAGROSS - 1]     = ANIM_V_SHAKE,
    [SPECIES_REGIROCK - 1]      = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_REGICE - 1]        = ANIM_FOUR_PETAL,
    [SPECIES_REGISTEEL - 1]     = ANIM_GROW_VIBRATE,
    [SPECIES_LATIAS - 1]        = ANIM_SWING_CONCAVE_FAST_SHORT,
    [SPECIES_LATIOS - 1]        = ANIM_V_SHAKE,
    [SPECIES_KYOGRE - 1]        = ANIM_SWING_CONCAVE_FAST_SHORT,
    [SPECIES_GROUDON - 1]       = ANIM_V_SHAKE,
    [SPECIES_RAYQUAZA - 1]      = ANIM_H_SHAKE,
    [SPECIES_JIRACHI - 1]       = ANIM_SWING_CONVEX,
    [SPECIES_DEOXYS - 1]        = ANIM_H_PIVOT,

    // Gen 4
    [SPECIES_TURTWIG - 1]       = ANIM_V_SLIDE,
    [SPECIES_GROTLE - 1]        = ANIM_H_SLIDE,
    [SPECIES_TORTERRA - 1]      = ANIM_V_SHAKE,
    [SPECIES_CHIMCHAR - 1]      = ANIM_V_JUMPS_BIG,
    [SPECIES_MONFERNO - 1]      = ANIM_H_SHAKE_SLOW,
    [SPECIES_INFERNAPE - 1]     = ANIM_BACK_AND_LUNGE,
    [SPECIES_PIPLUP - 1]        = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_PRINPLUP - 1]      = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_EMPOLEON - 1]      = ANIM_V_SHAKE_TWICE,
    [SPECIES_STARLY - 1]        = ANIM_V_STRETCH,
    [SPECIES_STARAVIA - 1]      = ANIM_V_STRETCH,
    [SPECIES_STARAPTOR - 1]     = ANIM_H_STRETCH,
    [SPECIES_BIDOOF - 1]        = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_BIBAREL - 1]       = ANIM_GROW,
    [SPECIES_KRICKETOT - 1]     = ANIM_V_SHAKE,
    [SPECIES_KRICKETUNE - 1]    = ANIM_V_SHAKE,
    [SPECIES_SHINX - 1]         = ANIM_V_STRETCH,
    [SPECIES_LUXIO - 1]         = ANIM_V_STRETCH,
    [SPECIES_LUXRAY - 1]        = ANIM_GLOW_YELLOW,
    [SPECIES_BUDEW - 1]         = ANIM_SHRINK_GROW,
    [SPECIES_ROSERADE - 1]      = ANIM_GLOW_PURPLE,
    [SPECIES_CRANIDOS - 1]      = ANIM_V_SHAKE_TWICE,
    [SPECIES_RAMPARDOS - 1]     = ANIM_V_SHAKE_TWICE,
    [SPECIES_SHIELDON - 1]      = ANIM_V_SHAKE,
    [SPECIES_BASTIODON - 1]     = ANIM_H_SHAKE,
    [SPECIES_BURMY - 1]         = ANIM_V_JUMPS_BIG,
    [SPECIES_WORMADAM - 1]      = ANIM_V_JUMPS_BIG,
    [SPECIES_MOTHIM - 1]        = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_COMBEE - 1]        = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_VESPIQUEN - 1]     = ANIM_GLOW_ORANGE,
    [SPECIES_PACHIRISU - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BUIZEL - 1]        = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_FLOATZEL - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_CHERUBI - 1]       = ANIM_DEEP_V_SQUISH_AND_BOUNCE,
    [SPECIES_CHERRIM - 1]       = ANIM_DEEP_V_SQUISH_AND_BOUNCE,
    [SPECIES_SHELLOS - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_GASTRODON - 1]     = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_AMBIPOM - 1]       = ANIM_BACK_AND_LUNGE,
    [SPECIES_DRIFLOON - 1]      = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_DRIFBLIM - 1]      = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_BUNEARY - 1]       = ANIM_V_JUMPS_BIG,
    [SPECIES_LOPUNNY - 1]       = ANIM_SHRINK_GROW,
    [SPECIES_MISMAGIUS - 1]     = ANIM_SWING_CONVEX_FAST,
    [SPECIES_HONCHKROW - 1]     = ANIM_GLOW_BLACK,
    [SPECIES_GLAMEOW - 1]       = ANIM_GROW_VIBRATE,
    [SPECIES_PURUGLY - 1]       = ANIM_V_SHAKE,
    [SPECIES_CHINGLING - 1]     = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_STUNKY - 1]        = ANIM_TIP_MOVE_FORWARD,
    [SPECIES_SKUNTANK - 1]      = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_BRONZOR - 1]       = ANIM_TWIST_TWICE,
    [SPECIES_BRONZONG - 1]      = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_BONSLY - 1]        = ANIM_V_JUMPS_SMALL,
    [SPECIES_MIME_JR - 1]       = ANIM_H_SLIDE_SHRINK,
    [SPECIES_HAPPINY - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_CHATOT - 1]        = ANIM_V_SHAKE_TWICE,
    [SPECIES_SPIRITOMB - 1]     = ANIM_GROW_IN_STAGES,
    [SPECIES_GIBLE - 1]         = ANIM_V_JUMPS_BIG,
    [SPECIES_GABITE - 1]        = ANIM_V_STRETCH,
    [SPECIES_GARCHOMP - 1]      = ANIM_V_SHAKE_TWICE,
    [SPECIES_MUNCHLAX - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_RIOLU - 1]         = ANIM_H_STRETCH,
    [SPECIES_LUCARIO - 1]       = ANIM_H_SHAKE,
    [SPECIES_HIPPOPOTAS - 1]    = ANIM_V_STRETCH,
    [SPECIES_HIPPOWDON - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SKORUPI - 1]       = ANIM_V_SHAKE,
    [SPECIES_DRAPION - 1]       = ANIM_V_JUMPS_BIG,
    [SPECIES_CROAGUNK - 1]      = ANIM_GLOW_PURPLE,
    [SPECIES_TOXICROAK - 1]     = ANIM_V_SLIDE,
    [SPECIES_CARNIVINE - 1]     = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_FINNEON - 1]       = ANIM_SWING_CONCAVE_FAST_SHORT,
    [SPECIES_LUMINEON - 1]      = ANIM_GLOW_BLUE,
    [SPECIES_MANTYKE - 1]       = ANIM_TWIST_TWICE,
    [SPECIES_SNOVER - 1]        = ANIM_V_JUMPS_SMALL,
    [SPECIES_ABOMASNOW - 1]     = ANIM_H_SHAKE,
    [SPECIES_WEAVILE - 1]       = ANIM_H_SHAKE,
    [SPECIES_MAGNEZONE - 1]     = ANIM_GLOW_YELLOW,
    [SPECIES_LICKILICKY - 1]    = ANIM_ROTATE_UP_SLAM_DOWN,
    [SPECIES_RHYPERIOR - 1]     = ANIM_LUNGE_GROW,
    [SPECIES_TANGROWTH - 1]     = ANIM_GROW_IN_STAGES,
    [SPECIES_ELECTIVIRE - 1]    = ANIM_GLOW_YELLOW,
    [SPECIES_MAGMORTAR - 1]     = ANIM_GLOW_RED,
    [SPECIES_TOGEKISS - 1]      = ANIM_TIP_MOVE_FORWARD,
    [SPECIES_YANMEGA - 1]       = ANIM_ZIGZAG_FAST,
    [SPECIES_LEAFEON - 1]       = ANIM_V_JUMPS_SMALL,
    [SPECIES_GLACEON - 1]       = ANIM_V_STRETCH,
    [SPECIES_GLISCOR - 1]       = ANIM_TWIST,
    [SPECIES_MAMOSWINE - 1]     = ANIM_V_JUMPS_BIG,
    [SPECIES_PORYGON_Z - 1]     = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_GALLADE - 1]       = ANIM_SHRINK_GROW,
    [SPECIES_PROBOPASS - 1]     = ANIM_SWING_CONVEX_FAST,
    [SPECIES_DUSKNOIR - 1]      = ANIM_GLOW_BLACK,
    [SPECIES_FROSLASS - 1]      = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_ROTOM - 1]         = ANIM_GLOW_YELLOW,
    [SPECIES_UXIE - 1]          = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_MESPRIT - 1]       = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_AZELF - 1]         = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_DIALGA - 1]        = ANIM_DEEP_V_SQUISH_AND_BOUNCE,
    [SPECIES_PALKIA - 1]        = ANIM_H_SHAKE,
    [SPECIES_HEATRAN - 1]       = ANIM_GLOW_RED,
    [SPECIES_REGIGIGAS - 1]     = ANIM_GROW_IN_STAGES,
    [SPECIES_GIRATINA - 1]      = ANIM_V_SHAKE_TWICE,
    [SPECIES_CRESSELIA - 1]     = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_PHIONE - 1]        = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_MANAPHY - 1]       = ANIM_V_STRETCH,
    [SPECIES_DARKRAI - 1]       = ANIM_GLOW_BLACK,
    [SPECIES_SHAYMIN - 1]       = ANIM_SHRINK_GROW,
    [SPECIES_ARCEUS - 1]        = ANIM_CIRCULAR_VIBRATE,

    // Gen 5 Todo: Assign proper ones.
    [SPECIES_VICTINI - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SNIVY - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SERVINE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SERPERIOR - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TEPIG - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PIGNITE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_EMBOAR - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_OSHAWOTT - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DEWOTT - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SAMUROTT - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PATRAT - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_WATCHOG - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LILLIPUP - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_HERDIER - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_STOUTLAND - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PURRLOIN - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LIEPARD - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PANSAGE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SIMISAGE - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PANSEAR - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SIMISEAR - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PANPOUR - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SIMIPOUR - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MUNNA - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MUSHARNA - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PIDOVE - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TRANQUILL - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_UNFEZANT - 1]      = ANIM_V_STRETCH,
    [SPECIES_BLITZLE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ZEBSTRIKA - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ROGGENROLA - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BOLDORE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GIGALITH - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_WOOBAT - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SWOOBAT - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DRILBUR - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_EXCADRILL - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_AUDINO - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TIMBURR - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GURDURR - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CONKELDURR - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TYMPOLE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PALPITOAD - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SEISMITOAD - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_THROH - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SAWK - 1]          = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SEWADDLE - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SWADLOON - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LEAVANNY - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_VENIPEDE - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_WHIRLIPEDE - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SCOLIPEDE - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_COTTONEE - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_WHIMSICOTT - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PETILIL - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LILLIGANT - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BASCULIN - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SANDILE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_KROKOROK - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_KROOKODILE - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DARUMAKA - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DARMANITAN - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MARACTUS - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DWEBBLE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CRUSTLE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SCRAGGY - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SCRAFTY - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SIGILYPH - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_YAMASK - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_COFAGRIGUS - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TIRTOUGA - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CARRACOSTA - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ARCHEN - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ARCHEOPS - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TRUBBISH - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GARBODOR - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ZORUA - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ZOROARK - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MINCCINO - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CINCCINO - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GOTHITA - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GOTHORITA - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GOTHITELLE - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SOLOSIS - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DUOSION - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_REUNICLUS - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DUCKLETT - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SWANNA - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_VANILLITE - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_VANILLISH - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_VANILLUXE - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DEERLING - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SAWSBUCK - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_EMOLGA - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_KARRABLAST - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ESCAVALIER - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FOONGUS - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_AMOONGUSS - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FRILLISH - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_JELLICENT - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ALOMOMOLA - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_JOLTIK - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GALVANTULA - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FERROSEED - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FERROTHORN - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_KLINK - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_KLANG - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_KLINKLANG - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TYNAMO - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_EELEKTRIK - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_EELEKTROSS - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ELGYEM - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BEHEEYEM - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LITWICK - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LAMPENT - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CHANDELURE - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_AXEW - 1]          = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FRAXURE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_HAXORUS - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CUBCHOO - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BEARTIC - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CRYOGONAL - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SHELMET - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ACCELGOR - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_STUNFISK - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MIENFOO - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MIENSHAO - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DRUDDIGON - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GOLETT - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GOLURK - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PAWNIARD - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BISHARP - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BOUFFALANT - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_RUFFLET - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BRAVIARY - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_VULLABY - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MANDIBUZZ - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_HEATMOR - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DURANT - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DEINO - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ZWEILOUS - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_HYDREIGON - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LARVESTA - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_VOLCARONA - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_COBALION - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TERRAKION - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_VIRIZION - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TORNADUS - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_THUNDURUS - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_RESHIRAM - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ZEKROM - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_LANDORUS - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_KYUREM - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_KELDEO - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MELOETTA - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GENESECT - 1]      = ANIM_V_SQUISH_AND_BOUNCE,

    //Gen 6
    [SPECIES_CHESPIN - 1]       = ANIM_H_JUMPS,
    [SPECIES_QUILLADIN - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CHESNAUGHT - 1]    = ANIM_V_SHAKE_TWICE,
    [SPECIES_FENNEKIN - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BRAIXEN - 1]       = ANIM_SHAKE_GLOW_RED_SLOW,
    [SPECIES_DELPHOX - 1]       = ANIM_V_STRETCH,
    [SPECIES_FROAKIE - 1]       = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_FROGADIER - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GRENINJA - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_BUNNELBY - 1]      = ANIM_V_JUMPS_SMALL,
    [SPECIES_DIGGERSBY - 1]     = ANIM_V_SHAKE_TWICE,
    [SPECIES_FLETCHLING - 1]    = ANIM_H_STRETCH,
    [SPECIES_FLETCHINDER - 1]   = ANIM_GROW_VIBRATE,
    [SPECIES_TALONFLAME - 1]    = ANIM_GROW_IN_STAGES,
    [SPECIES_SCATTERBUG - 1]    = ANIM_H_JUMPS,
    [SPECIES_SPEWPA - 1]        = ANIM_V_SHAKE,
    [SPECIES_VIVILLON - 1]      = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_LITLEO - 1]        = ANIM_BACK_AND_LUNGE,
    [SPECIES_PYROAR - 1]        = ANIM_GROW_VIBRATE,
    [SPECIES_FLABEBE - 1]       = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_FLOETTE - 1]       = ANIM_SWING_CONVEX,
    [SPECIES_FLORGES - 1]       = ANIM_V_SLIDE_SLOW,
    [SPECIES_SKIDDO - 1]        = ANIM_H_STRETCH,
    [SPECIES_GOGOAT - 1]        = ANIM_V_STRETCH,
    [SPECIES_PANCHAM - 1]       = ANIM_V_JUMPS_SMALL,
    [SPECIES_PANGORO - 1]       = ANIM_V_SHAKE_TWICE,
    [SPECIES_FURFROU - 1]       = ANIM_H_STRETCH,
    [SPECIES_ESPURR - 1]        = ANIM_H_STRETCH,
    [SPECIES_MEOWSTIC - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_HONEDGE - 1]       = ANIM_SWING_CONVEX,
    [SPECIES_DOUBLADE - 1]      = ANIM_SWING_CONVEX,
    [SPECIES_AEGISLASH - 1]     = ANIM_H_VIBRATE,
    [SPECIES_SPRITZEE - 1]      = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_AROMATISSE - 1]    = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_SWIRLIX - 1]       = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_SLURPUFF - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_INKAY - 1]         = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_MALAMAR - 1]       = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_BINACLE - 1]       = ANIM_V_STRETCH,
    [SPECIES_BARBARACLE - 1]    = ANIM_ROTATE_UP_SLAM_DOWN,
    [SPECIES_SKRELP - 1]        = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_DRAGALGE - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_CLAUNCHER - 1]     = ANIM_V_JUMPS_SMALL,
    [SPECIES_CLAWITZER - 1]     = ANIM_BACK_AND_LUNGE,
    [SPECIES_HELIOPTILE - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_HELIOLISK - 1]     = ANIM_GROW_VIBRATE,
    [SPECIES_TYRUNT - 1]        = ANIM_V_SHAKE,
    [SPECIES_TYRANTRUM - 1]     = ANIM_H_SHAKE,
    [SPECIES_AMAURA - 1]        = ANIM_V_STRETCH,
    [SPECIES_AURORUS - 1]       = ANIM_V_SHAKE_TWICE,
    [SPECIES_SYLVEON - 1]       = ANIM_H_STRETCH,
    [SPECIES_HAWLUCHA - 1]      = ANIM_H_STRETCH,
    [SPECIES_DEDENNE - 1]       = ANIM_V_JUMPS_SMALL,
    [SPECIES_CARBINK - 1]       = ANIM_SWING_CONVEX,
    [SPECIES_GOOMY - 1]         = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_SLIGGOO - 1]       = ANIM_H_SPRING_SLOW,
    [SPECIES_GOODRA - 1]        = ANIM_V_SHAKE,
    [SPECIES_KLEFKI - 1]        = ANIM_SWING_CONVEX,
    [SPECIES_PHANTUMP - 1]      = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_TREVENANT - 1]     = ANIM_FLICKER_INCREASING,
    [SPECIES_PUMPKABOO - 1]     = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_GOURGEIST - 1]     = ANIM_SWING_CONVEX,
    [SPECIES_BERGMITE - 1]      = ANIM_V_SHAKE,
    [SPECIES_AVALUGG - 1]       = ANIM_ROTATE_UP_SLAM_DOWN,
    [SPECIES_NOIBAT - 1]        = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_NOIVERN - 1]       = ANIM_GROW_VIBRATE,
    [SPECIES_XERNEAS - 1]       = ANIM_GLOW_YELLOW,
    [SPECIES_YVELTAL - 1]       = ANIM_SHAKE_GLOW_BLACK_SLOW,
    [SPECIES_ZYGARDE - 1]       = ANIM_GROW_VIBRATE,
    [SPECIES_DIANCIE - 1]       = ANIM_SWING_CONVEX,
    [SPECIES_HOOPA - 1]         = ANIM_ZIGZAG_SLOW,
    [SPECIES_VOLCANION - 1]     = ANIM_V_SHAKE,

    // Gen 7
    [SPECIES_ROWLET - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DARTRIX - 1]       = ANIM_H_STRETCH,
    [SPECIES_DECIDUEYE - 1]     = ANIM_H_VIBRATE,
    [SPECIES_LITTEN - 1]        = ANIM_H_STRETCH,
    [SPECIES_TORRACAT - 1]      = ANIM_V_STRETCH,
    [SPECIES_INCINEROAR - 1]    = ANIM_V_SHAKE,
    [SPECIES_POPPLIO - 1]       = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_BRIONNE - 1]       = ANIM_SHRINK_GROW,
    [SPECIES_PRIMARINA - 1]     = ANIM_SHAKE_GLOW_BLUE_SLOW,
    [SPECIES_PIKIPEK - 1]       = ANIM_V_JUMPS_SMALL,
    [SPECIES_TRUMBEAK - 1]      = ANIM_H_STRETCH,
    [SPECIES_TOUCANNON - 1]     = ANIM_SHRINK_GROW_VIBRATE_FAST,
    [SPECIES_YUNGOOS - 1]       = ANIM_V_STRETCH,
    [SPECIES_GUMSHOOS - 1]      = ANIM_H_SHAKE,
    [SPECIES_GRUBBIN - 1]       = ANIM_H_SLIDE,
    [SPECIES_CHARJABUG - 1]     = ANIM_SHAKE_FLASH_YELLOW_SLOW,
    [SPECIES_VIKAVOLT - 1]      = ANIM_H_VIBRATE,
    [SPECIES_CRABRAWLER - 1]    = ANIM_V_SHAKE,
    [SPECIES_CRABOMINABLE - 1]  = ANIM_SHRINK_GROW_VIBRATE_FAST,
    [SPECIES_ORICORIO - 1]      = ANIM_CONCAVE_ARC_SMALL,
    [SPECIES_CUTIEFLY - 1]      = ANIM_V_SLIDE_WOBBLE,
    [SPECIES_RIBOMBEE - 1]      = ANIM_CONVEX_DOUBLE_ARC_TWICE,
    [SPECIES_ROCKRUFF - 1]      = ANIM_V_STRETCH,
    [SPECIES_LYCANROC - 1]      = ANIM_V_SHAKE,
    [SPECIES_WISHIWASHI - 1]    = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_MAREANIE - 1]      = ANIM_GLOW_PURPLE,
    [SPECIES_TOXAPEX - 1]       = ANIM_SHAKE_GLOW_PURPLE_SLOW,
    [SPECIES_MUDBRAY - 1]       = ANIM_V_SHAKE,
    [SPECIES_MUDSDALE - 1]      = ANIM_V_SHAKE_TWICE,
    [SPECIES_DEWPIDER - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_ARAQUANID - 1]     = ANIM_H_SHAKE,
    [SPECIES_FOMANTIS - 1]      = ANIM_V_SQUISH_AND_BOUNCE_SLOW,
    [SPECIES_LURANTIS - 1]      = ANIM_GROW_VIBRATE,
    [SPECIES_MORELULL - 1]      = ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_SHIINOTIC - 1]     = ANIM_SHAKE_GLOW_WHITE_SLOW,
    [SPECIES_SALANDIT - 1]      = ANIM_SHAKE_GLOW_RED_SLOW,
    [SPECIES_SALAZZLE - 1]      = ANIM_GROW_VIBRATE,
    [SPECIES_STUFFUL - 1]       = ANIM_H_STRETCH,
    [SPECIES_BEWEAR - 1]        = ANIM_H_STRETCH,
    [SPECIES_BOUNSWEET - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_STEENEE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TSAREENA - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_COMFEY - 1]        = ANIM_H_SLIDE_WOBBLE,
    [SPECIES_ORANGURU - 1]      = ANIM_SHRINK_GROW,
    [SPECIES_PASSIMIAN - 1]     = ANIM_V_SHAKE,
    [SPECIES_WIMPOD - 1]        = ANIM_V_SHAKE_H_SLIDE_FAST,
    [SPECIES_GOLISOPOD - 1]     = ANIM_SHRINK_GROW_VIBRATE_FAST,
    [SPECIES_SANDYGAST - 1]     = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_PALOSSAND - 1]     = ANIM_H_VIBRATE,
    [SPECIES_PYUKUMUKU - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TYPE_NULL - 1]     = ANIM_H_SHAKE,
    [SPECIES_SILVALLY - 1]      = ANIM_V_SHAKE,
    [SPECIES_MINIOR - 1]        = ANIM_TUMBLING_FRONT_FLIP_TWICE,
    [SPECIES_KOMALA - 1]        = ANIM_H_DIP,
    [SPECIES_TURTONATOR - 1]    = ANIM_SHAKE_GLOW_RED,
    [SPECIES_TOGEDEMARU - 1]    = ANIM_SHAKE_FLASH_YELLOW,
    [SPECIES_MIMIKYU - 1]       = ANIM_DEEP_V_SQUISH_AND_BOUNCE,
    [SPECIES_BRUXISH - 1]       = ANIM_RISING_WOBBLE,
    [SPECIES_DRAMPA - 1]        = ANIM_V_SHAKE,
    [SPECIES_DHELMISE - 1]      = ANIM_SWING_CONVEX,
    [SPECIES_JANGMO_O - 1]      = ANIM_H_STRETCH,
    [SPECIES_HAKAMO_O - 1]      = ANIM_H_STRETCH,
    [SPECIES_KOMMO_O - 1]       = ANIM_V_SHAKE_TWICE,
    [SPECIES_TAPU_KOKO - 1]     = ANIM_TRIANGLE_DOWN_TWICE,
    [SPECIES_TAPU_LELE - 1]     = ANIM_V_SLIDE_WOBBLE_SMALL,
    [SPECIES_TAPU_BULU - 1]     = ANIM_V_SHAKE_TWICE,
    [SPECIES_TAPU_FINI - 1]     = ANIM_SHAKE_GLOW_BLUE_SLOW,
    [SPECIES_COSMOG - 1]        = ANIM_H_JUMPS_V_STRETCH,
    [SPECIES_COSMOEM - 1]       = ANIM_RISING_WOBBLE,
    [SPECIES_SOLGALEO - 1]      = ANIM_GROW_VIBRATE,
    [SPECIES_LUNALA - 1]        = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_NIHILEGO - 1]      = ANIM_CIRCULAR_STRETCH_TWICE,
    [SPECIES_BUZZWOLE - 1]      = ANIM_SHRINK_GROW_VIBRATE_FAST,
    [SPECIES_PHEROMOSA - 1]     = ANIM_SHRINK_GROW,
    [SPECIES_XURKITREE - 1]     = ANIM_SHAKE_FLASH_YELLOW_FAST,
    [SPECIES_CELESTEELA - 1]    = ANIM_GROW_STUTTER_SLOW,
    [SPECIES_KARTANA - 1]       = ANIM_H_VIBRATE,
    [SPECIES_GUZZLORD - 1]      = ANIM_SHRINK_GROW_VIBRATE_FAST,
    [SPECIES_NECROZMA - 1]      = ANIM_GROW_VIBRATE,
    [SPECIES_MAGEARNA - 1]      = ANIM_H_SLIDE_SLOW,
    [SPECIES_MARSHADOW - 1]     = ANIM_V_SHAKE,
    [SPECIES_POIPOLE - 1]       = ANIM_SHAKE_GLOW_PURPLE_SLOW,
    [SPECIES_NAGANADEL - 1]     = ANIM_TRIANGLE_DOWN_TWICE,
    [SPECIES_STAKATAKA - 1]     = ANIM_GROW_VIBRATE,
    [SPECIES_BLACEPHALON - 1]   = ANIM_SHAKE_GLOW_RED,
    [SPECIES_ZERAORA - 1]       = ANIM_V_STRETCH,
    [SPECIES_MELTAN - 1]        = ANIM_GROW_STUTTER_SLOW,
    [SPECIES_MELMETAL - 1]      = ANIM_GROW_VIBRATE,

    // Gen 8 Todo: Assign proper ones.
    [SPECIES_GROOKEY - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_THWACKEY - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_RILLABOOM - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SCORBUNNY - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_RABOOT - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CINDERACE - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SOBBLE - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DRIZZILE - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_INTELEON - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SKWOVET - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GREEDENT - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ROOKIDEE - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CORVISQUIRE - 1]   = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CORVIKNIGHT - 1]   = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BLIPBUG - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DOTTLER - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ORBEETLE - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_NICKIT - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_THIEVUL - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GOSSIFLEUR - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ELDEGOSS - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_WOOLOO - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DUBWOOL - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CHEWTLE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DREDNAW - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_YAMPER - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BOLTUND - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ROLYCOLY - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CARKOL - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_COALOSSAL - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_APPLIN - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FLAPPLE - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_APPLETUN - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SILICOBRA - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SANDACONDA - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CRAMORANT - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ARROKUDA - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_BARRASKEWDA - 1]   = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TOXEL - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_TOXTRICITY - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SIZZLIPEDE - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CENTISKORCH - 1]   = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CLOBBOPUS - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GRAPPLOCT - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SINISTEA - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_POLTEAGEIST - 1]   = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_HATENNA - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_HATTREM - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_HATTERENE - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_IMPIDIMP - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MORGREM - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GRIMMSNARL - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_OBSTAGOON - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PERRSERKER - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CURSOLA - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SIRFETCHD - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MR_RIME - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_RUNERIGUS - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MILCERY - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ALCREMIE - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FALINKS - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_PINCURCHIN - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SNOM - 1]          = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_FROSMOTH - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_STONJOURNER - 1]   = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_EISCUE - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_INDEEDEE - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_MORPEKO - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CUFANT - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_COPPERAJAH - 1]    = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DRACOZOLT - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ARCTOZOLT - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DRACOVISH - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ARCTOVISH - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DURALUDON - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DREEPY - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DRAKLOAK - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_DRAGAPULT - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ZACIAN - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ZAMAZENTA - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ETERNATUS - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_KUBFU - 1]         = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_URSHIFU - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ZARUDE - 1]        = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_REGIELEKI - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_REGIDRAGO - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_GLASTRIER - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_SPECTRIER - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_CALYREX - 1]       = ANIM_V_SQUISH_AND_BOUNCE,

    // Forms
    [SPECIES_ROTOM_HEAT - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ROTOM_FROST - 1]     = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ROTOM_FAN - 1]       = ANIM_FIGURE_8,
    [SPECIES_ROTOM_MOW - 1]       = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ROTOM_WASH - 1]      = ANIM_V_SQUISH_AND_BOUNCE,
    [SPECIES_ARCEUS_FIGHTING - 1] = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_FLYING - 1]   = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_POISON - 1]   = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_GROUND - 1]   = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_ROCK - 1]     = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_BUG - 1]      = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_GHOST - 1]    = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_STEEL - 1]    = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_FIRE - 1]     = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_WATER - 1]    = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_GRASS - 1]    = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_ELECTRIC - 1] = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_PSYCHIC - 1]  = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_ICE - 1]      = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_DRAGON - 1]   = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_DARK - 1]     = ANIM_CIRCULAR_VIBRATE,
    [SPECIES_ARCEUS_FAIRY - 1]    = ANIM_CIRCULAR_VIBRATE,
};

static const u8 sMonAnimationDelayTable[NUM_SPECIES - 1] =
{
    [SPECIES_BLASTOISE - 1]  = 50,
    [SPECIES_WEEDLE - 1]     = 10,
    [SPECIES_KAKUNA - 1]     = 20,
    [SPECIES_BEEDRILL - 1]   = 35,
    [SPECIES_PIDGEOTTO - 1]  = 25,
    [SPECIES_FEAROW - 1]     = 2,
    [SPECIES_EKANS - 1]      = 30,
    [SPECIES_NIDORAN_F - 1]  = 28,
    [SPECIES_NIDOKING - 1]   = 25,
    [SPECIES_PARAS - 1]      = 10,
    [SPECIES_PARASECT - 1]   = 45,
    [SPECIES_VENONAT - 1]    = 20,
    [SPECIES_DIGLETT - 1]    = 25,
    [SPECIES_DUGTRIO - 1]    = 35,
    [SPECIES_MEOWTH - 1]     = 40,
    [SPECIES_PERSIAN - 1]    = 20,
    [SPECIES_MANKEY - 1]     = 20,
    [SPECIES_GROWLITHE - 1]  = 30,
    [SPECIES_ARCANINE - 1]   = 40,
    [SPECIES_POLIWHIRL - 1]  = 5,
    [SPECIES_WEEPINBELL - 1] = 3,
    [SPECIES_MUK - 1]        = 45,
    [SPECIES_SHELLDER - 1]   = 20,
    [SPECIES_HAUNTER - 1]    = 23,
    [SPECIES_DROWZEE - 1]    = 48,
    [SPECIES_HYPNO - 1]      = 40,
    [SPECIES_HITMONCHAN - 1] = 25,
    [SPECIES_SCYTHER - 1]    = 10,
    [SPECIES_TAUROS - 1]     = 10,
    [SPECIES_TYPHLOSION - 1] = 20,
    [SPECIES_FERALIGATR - 1] = 5,
    [SPECIES_NATU - 1]       = 30,
    [SPECIES_MAREEP - 1]     = 50,
    [SPECIES_AMPHAROS - 1]   = 10,
    [SPECIES_POLITOED - 1]   = 40,
    [SPECIES_DUNSPARCE - 1]  = 10,
    [SPECIES_STEELIX - 1]    = 45,
    [SPECIES_QWILFISH - 1]   = 39,
    [SPECIES_SCIZOR - 1]     = 19,
    [SPECIES_OCTILLERY - 1]  = 20,
    [SPECIES_SMOOCHUM - 1]   = 40,
    [SPECIES_TYRANITAR - 1]  = 10,
    [SPECIES_LUGIA - 1]      = 20,
    [SPECIES_WAILORD - 1]    = 10,
    [SPECIES_KECLEON - 1]    = 30,
    [SPECIES_MILOTIC - 1]    = 45,
    [SPECIES_SPHEAL - 1]     = 15,
    [SPECIES_SNORUNT - 1]    = 20,
    [SPECIES_GRUMPIG - 1]    = 15,
    [SPECIES_WYNAUT - 1]     = 15,
    [SPECIES_DUSCLOPS - 1]   = 30,
    [SPECIES_ABSOL - 1]      = 45,
    [SPECIES_SALAMENCE - 1]  = 70,
    [SPECIES_KYOGRE - 1]     = 60,
    [SPECIES_RAYQUAZA - 1]   = 60,
    [SPECIES_TAPU_FINI - 1]  = 5,
    [SPECIES_ROTOM_FAN - 1]  = 7,
};

static const union AffineAnimCmd sMonAffineAnim_0[] =
{
    AFFINEANIMCMD_FRAME(256, 256, 0, 0),
    AFFINEANIMCMDTYPE_END
};

static const union AffineAnimCmd sMonAffineAnim_1[] =
{
    AFFINEANIMCMD_FRAME(-256, 256, 0, 0),
    AFFINEANIMCMDTYPE_END
};

static const union AffineAnimCmd *const sMonAffineAnims[] =
{
    sMonAffineAnim_0,
    sMonAffineAnim_1
};

static void MonAnimDummySpriteCallback(struct Sprite *sprite)
{
}

static void SetPosForRotation(struct Sprite *sprite, u16 index, s16 amplitudeX, s16 amplitudeY)
{
    s16 xAdder, yAdder;

    amplitudeX *= -1;
    amplitudeY *= -1;

    xAdder = Cos(index, amplitudeX) - Sin(index, amplitudeY);
    yAdder = Cos(index, amplitudeY) + Sin(index, amplitudeX);

    amplitudeX *= -1;
    amplitudeY *= -1;

    sprite->x2 = xAdder + amplitudeX;
    sprite->y2 = yAdder + amplitudeY;
}

u8 GetSpeciesBackAnimSet(u16 species)
{
    if (sSpeciesToBackAnimSet[species] != BACK_ANIM_NONE)
        return sSpeciesToBackAnimSet[species] - 1;
    else
        return 0;
}

u8 GetSpeciesFrontAnimSet(u16 species)
{
    return sMonFrontAnimIdsTable[species - 1];
}

u8 GetSpeciesFrontAnimDelay(u16 species)
{
    return sMonAnimationDelayTable[species - 1];
}

#define tState  data[0]
#define tPtrHi  data[1]
#define tPtrLo  data[2]
#define tAnimId data[3]
#define tBattlerId data[4]
#define tSpeciesId data[5]

// BUG: In vanilla, tPtrLo is read as an s16, so if bit 15 of the
// address were to be set it would cause the pointer to be read
// as 0xFFFFXXXX instead of the desired 0x02YYXXXX.
// By dumb luck, this is not an issue in vanilla. However,
// changing the link order revealed this bug.
#if MODERN
#define ANIM_SPRITE(taskId)   ((struct Sprite *)((gTasks[taskId].tPtrHi << 16) | ((u16)gTasks[taskId].tPtrLo)))
#else
#define ANIM_SPRITE(taskId)   ((struct Sprite *)((gTasks[taskId].tPtrHi << 16) | (gTasks[taskId].tPtrLo)))
#endif //MODERN

static void Task_HandleMonAnimation(u8 taskId)
{
    u32 i;
    struct Sprite *sprite = ANIM_SPRITE(taskId);

    if (gTasks[taskId].tState == 0)
    {
        gTasks[taskId].tBattlerId = sprite->data[0];
        gTasks[taskId].tSpeciesId = sprite->data[2];
        sprite->sDontFlip = TRUE;
        sprite->data[0] = 0;

        for (i = 2; i < ARRAY_COUNT(sprite->data); i++)
            sprite->data[i] = 0;

        sprite->callback = sMonAnimFunctions[gTasks[taskId].tAnimId];
        sIsSummaryAnim = FALSE;

        gTasks[taskId].tState++;
    }
    if (sprite->callback == SpriteCallbackDummy)
    {
        sprite->data[0] = gTasks[taskId].tBattlerId;
        sprite->data[2] = gTasks[taskId].tSpeciesId;
        sprite->data[1] = 0;

        DestroyTask(taskId);
    }
}

void LaunchAnimationTaskForFrontSprite(struct Sprite *sprite, u8 frontAnimId)
{
    u8 taskId = CreateTask(Task_HandleMonAnimation, 128);
    gTasks[taskId].tPtrHi = (u32)(sprite) >> 16;
    gTasks[taskId].tPtrLo = (u32)(sprite);
    gTasks[taskId].tAnimId = frontAnimId;
}

void StartMonSummaryAnimation(struct Sprite *sprite, u8 frontAnimId)
{
    // sDontFlip is expected to still be FALSE here, not explicitly cleared
    sIsSummaryAnim = TRUE;
    sprite->callback = sMonAnimFunctions[frontAnimId];
}

void LaunchAnimationTaskForBackSprite(struct Sprite *sprite, u8 backAnimSet)
{
    u8 nature, taskId, animId, battlerId;

    taskId = CreateTask(Task_HandleMonAnimation, 128);
    gTasks[taskId].tPtrHi = (u32)(sprite) >> 16;
    gTasks[taskId].tPtrLo = (u32)(sprite);

    battlerId = sprite->data[0];
    nature = GetNature(&gPlayerParty[gBattlerPartyIndexes[battlerId]]);

    // * 3 below because each back anim has 3 variants depending on nature
    animId = 3 * backAnimSet + sBackAnimNatureModTable[nature];
    gTasks[taskId].tAnimId = sBackAnimationIds[animId];
}

#undef tState
#undef tPtrHi
#undef tPtrLo
#undef tAnimId
#undef tBattlerId
#undef tSpeciesId

void SetSpriteCB_MonAnimDummy(struct Sprite *sprite)
{
    sprite->callback = MonAnimDummySpriteCallback;
}

static void SetAffineData(struct Sprite *sprite, s16 xScale, s16 yScale, u16 rotation)
{
    u8 matrixNum;
    struct ObjAffineSrcData affineSrcData;
    struct OamMatrix dest;

    affineSrcData.xScale = xScale;
    affineSrcData.yScale = yScale;
    affineSrcData.rotation = rotation;

    matrixNum = sprite->oam.matrixNum;

    ObjAffineSet(&affineSrcData, &dest, 1, 2);
    gOamMatrices[matrixNum].a = dest.a;
    gOamMatrices[matrixNum].b = dest.b;
    gOamMatrices[matrixNum].c = dest.c;
    gOamMatrices[matrixNum].d = dest.d;
}

static void HandleStartAffineAnim(struct Sprite *sprite)
{
    sprite->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
    sprite->affineAnims = sMonAffineAnims;

    if (sIsSummaryAnim == TRUE)
        InitSpriteAffineAnim(sprite);

    if (!sprite->sDontFlip)
        StartSpriteAffineAnim(sprite, 1);
    else
        StartSpriteAffineAnim(sprite, 0);

    CalcCenterToCornerVec(sprite, sprite->oam.shape, sprite->oam.size, sprite->oam.affineMode);
    sprite->affineAnimPaused = TRUE;
}

static void HandleSetAffineData(struct Sprite *sprite, s16 xScale, s16 yScale, u16 rotation)
{
    if (!sprite->sDontFlip)
    {
        xScale *= -1;
        rotation *= -1;
    }

    SetAffineData(sprite, xScale, yScale, rotation);
}

static void TryFlipX(struct Sprite *sprite)
{
    if (!sprite->sDontFlip)
        sprite->x2 *= -1;
}

static bool32 InitAnimData(u8 id)
{
    if (id >= MAX_BATTLERS_COUNT)
    {
        return FALSE;
    }
    else
    {
        sAnims[id].rotation = 0;
        sAnims[id].delay = 0;
        sAnims[id].runs = 1;
        sAnims[id].speed = 0;
        sAnims[id].data = 0;
        return TRUE;
    }
}

static u8 AddNewAnim(void)
{
    sAnimIdx = (sAnimIdx + 1) % MAX_BATTLERS_COUNT;
    InitAnimData(sAnimIdx);
    return sAnimIdx;
}

static void ResetSpriteAfterAnim(struct Sprite *sprite)
{
    sprite->oam.affineMode = ST_OAM_AFFINE_NORMAL;
    CalcCenterToCornerVec(sprite, sprite->oam.shape, sprite->oam.size, sprite->oam.affineMode);

    if (sIsSummaryAnim == TRUE)
    {
        if (!sprite->sDontFlip)
            sprite->hFlip = TRUE;
        else
            sprite->hFlip = FALSE;

        FreeOamMatrix(sprite->oam.matrixNum);
        sprite->oam.matrixNum |= (sprite->hFlip << 3);
        sprite->oam.affineMode = ST_OAM_AFFINE_OFF;
    }
#ifdef BUGFIX
    else
    {
        // FIX: Reset these back to normal after they were changed so Poké Ball catch/release
        // animations without a screen transition in between don't break
        sprite->affineAnims = gAffineAnims_BattleSpriteOpponentSide;
    }
#endif // BUGFIX
}

static void Anim_CircularStretchTwice(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > 40)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 var = (sprite->data[2] * 512 / 40) % 256;

        sprite->data[4] = Sin(var, 32) + 256;
        sprite->data[5] = Cos(var, 32) + 256;
        HandleSetAffineData(sprite, sprite->data[4], sprite->data[5], 0);
    }

    sprite->data[2]++;
}

static void Anim_HorizontalVibrate(struct Sprite *sprite)
{
    if (sprite->data[2] > 40)
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
    }
    else
    {
        s8 sign;
        if (!(sprite->data[2] & 1))
            sign = 1;
        else
            sign = -1;

        sprite->x2 = Sin((sprite->data[2] * 128 / 40) % 256, 6) * sign;
    }

    sprite->data[2]++;
}

static void HorizontalSlide(struct Sprite *sprite)
{
    TryFlipX(sprite);

    if (sprite->data[2] > sprite->data[0])
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
    }
    else
    {
        sprite->x2 = Sin((sprite->data[2] * 384 / sprite->data[0]) % 256, 6);
    }

    sprite->data[2]++;
    TryFlipX(sprite);
}

static void Anim_HorizontalSlide(struct Sprite *sprite)
{
    sprite->data[0] = 40;
    HorizontalSlide(sprite);
    sprite->callback = HorizontalSlide;
}

static void VerticalSlide(struct Sprite *sprite)
{
    TryFlipX(sprite);

    if (sprite->data[2] > sprite->data[0])
    {
        sprite->callback = WaitAnimEnd;
        sprite->y2 = 0;
    }
    else
    {
        sprite->y2 = -(Sin((sprite->data[2] * 384 / sprite->data[0]) % 256, 6));
    }

    sprite->data[2]++;
    TryFlipX(sprite);
}

static void Anim_VerticalSlide(struct Sprite *sprite)
{
    sprite->data[0] = 40;
    VerticalSlide(sprite);
    sprite->callback = VerticalSlide;
}

static void VerticalJumps(struct Sprite *sprite)
{
    s32 counter = sprite->data[2];
    if (counter > 384)
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
        sprite->y2 = 0;
    }
    else
    {
        s16 divCounter = counter / 128;
        switch (divCounter)
        {
        case 0:
        case 1:
            sprite->y2 = -(Sin(counter % 128, sprite->data[0] * 2));
            break;
        case 2:
        case 3:
            counter -= 256;
            sprite->y2 = -(Sin(counter, sprite->data[0] * 3));
            break;
        }
    }

    sprite->data[2] += 12;
}

static void Anim_VerticalJumps_Big(struct Sprite *sprite)
{
    sprite->data[0] = 4;
    VerticalJumps(sprite);
    sprite->callback = VerticalJumps;
}

static void Anim_VerticalJumpsHorizontalJumps(struct Sprite *sprite)
{
    s32 counter = sprite->data[2];

    if (counter > 768)
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
        sprite->y2 = 0;
    }
    else
    {
        s16 divCounter = counter / 128;

        switch (divCounter)
        {
        case 0:
        case 1:
            sprite->x2 = 0;
            break;
        case 2:
            counter = 0;
            break;
        case 3:
            sprite->x2 = -(counter % 128 * 8) / 128;
            break;
        case 4:
            sprite->x2 = (counter % 128) / 8 - 8;
            break;
        case 5:
            sprite->x2 = -(counter % 128 * 8) / 128 + 8;
            break;
        }

        sprite->y2 = -(Sin(counter % 128, 8));
    }

    sprite->data[2] += 12;
}

static void Anim_GrowVibrate(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > 40)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 index = (sprite->data[2] * 256 / 40) % 256;

        if (sprite->data[2] % 2 == 0)
        {
            sprite->data[4] = Sin(index, 32) + 256;
            sprite->data[5] = Sin(index, 32) + 256;
        }
        else
        {
            sprite->data[4] = Sin(index, 8) + 256;
            sprite->data[5] = Sin(index, 8) + 256;
        }

        HandleSetAffineData(sprite, sprite->data[4], sprite->data[5], 0);
    }

    sprite->data[2]++;
}

// x delta, y delta, time
static const s8 sZigzagData[][3] =
{
    {-1, -1, 6},
    { 2,  0, 6},
    {-2,  2, 6},
    { 2,  0, 6},
    {-2, -2, 6},
    { 2,  0, 6},
    {-2,  2, 6},
    { 2,  0, 6},
    {-1, -1, 6},
    { 0,  0, 0},
};

static void Zigzag(struct Sprite *sprite)
{
    TryFlipX(sprite);

    if (sprite->data[2] == 0)
        sprite->data[3] = 0;

    if (sZigzagData[sprite->data[3]][2] == sprite->data[2])
    {
        if (sZigzagData[sprite->data[3]][2] == 0)
        {
            sprite->callback = WaitAnimEnd;
        }
        else
        {
            sprite->data[3]++;
            sprite->data[2] = 0;
        }
    }

    if (sZigzagData[sprite->data[3]][2] == 0)
    {
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        sprite->x2 += sZigzagData[sprite->data[3]][0];
        sprite->y2 += sZigzagData[sprite->data[3]][1];
        sprite->data[2]++;
        TryFlipX(sprite);
    }
}

static void Anim_ZigzagFast(struct Sprite *sprite)
{
    Zigzag(sprite);
    sprite->callback = Zigzag;
}

static void HorizontalShake(struct Sprite *sprite)
{
    s32 counter = sprite->data[2];

    if (counter > 2304)
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
    }
    else
    {
        sprite->x2 = Sin(counter % 256, sprite->data[7]);
    }

    sprite->data[2] += sprite->data[0];
}

static void Anim_HorizontalShake(struct Sprite *sprite)
{
    sprite->data[0] = 60;
    sprite->data[7] = 3;
    HorizontalShake(sprite);
    sprite->callback = HorizontalShake;
}

static void VerticalShake(struct Sprite *sprite)
{
    s32 counter = sprite->data[2];

    if (counter > 2304)
    {
        sprite->callback = WaitAnimEnd;
        sprite->y2 = 0;
    }
    else
    {
        sprite->y2 = Sin(counter % 256, 3);
    }

    sprite->data[2] += sprite->data[0];
}

static void Anim_VerticalShake(struct Sprite *sprite)
{
    sprite->data[0] = 60;
    VerticalShake(sprite);
    sprite->callback = VerticalShake;
}

static void Anim_CircularVibrate(struct Sprite *sprite)
{
    if (sprite->data[2] > 512)
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
        sprite->y2 = 0;
    }
    else
    {
        s8 sign;
        s32 index, amplitude;

        if (!(sprite->data[2] & 1))
            sign = 1;
        else
            sign = -1;

        amplitude = Sin(sprite->data[2] / 4, 8);
        index = sprite->data[2] % 256;

        sprite->y2 = Sin(index, amplitude) * sign;
        sprite->x2 = Cos(index, amplitude) * sign;
    }

    sprite->data[2] += 9;
}

static void Twist(struct Sprite *sprite)
{
    s16 id = sprite->data[0];

    if (sAnims[id].delay != 0)
    {
        sAnims[id].delay--;
    }
    else
    {
        if (sprite->data[2] == 0 && sAnims[id].data == 0)
        {
            HandleStartAffineAnim(sprite);
            sAnims[id].data++;
        }

        if (sprite->data[2] > sAnims[id].rotation)
        {
            HandleSetAffineData(sprite, 256, 256, 0);

            if (sAnims[id].runs > 1)
            {
                sAnims[id].runs--;
                sAnims[id].delay = 10;
                sprite->data[2] = 0;
            }
            else
            {
                ResetSpriteAfterAnim(sprite);
                sprite->callback = WaitAnimEnd;
            }
        }
        else
        {
            sprite->data[6] = Sin(sprite->data[2] % 256, 4096);
            HandleSetAffineData(sprite, 256, 256, sprite->data[6]);
        }

        sprite->data[2] += 16;
    }
}

static void Anim_Twist(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].rotation = 512;
    sAnims[id].delay = 0;
    Twist(sprite);
    sprite->callback = Twist;
}

static void Spin(struct Sprite *sprite)
{
    u8 id = sprite->data[0];

    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > sAnims[id].delay)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        sprite->data[6] = (65536 / sAnims[id].data) * sprite->data[2];
        HandleSetAffineData(sprite, 256, 256, sprite->data[6]);
    }

    sprite->data[2]++;
}

static void Anim_Spin_Long(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].delay = 60;
    sAnims[id].data = 20;
    Spin(sprite);
    sprite->callback = Spin;
}

static void CircleCounterclockwise(struct Sprite *sprite)
{
    u8 id = sprite->data[0];

    TryFlipX(sprite);

    if (sprite->data[2] > sAnims[id].rotation)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 index = (sprite->data[2] + 192) % 256;

        sprite->x2 = -(Cos(index, sAnims[id].data * 2));
        sprite->y2 = Sin(index, sAnims[id].data) + sAnims[id].data;
    }

    sprite->data[2] += sAnims[id].speed;
    TryFlipX(sprite);
}

static void Anim_CircleCounterclockwise(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].rotation = 512;
    sAnims[id].data = 6;
    sAnims[id].speed = 24;
    CircleCounterclockwise(sprite);
    sprite->callback = CircleCounterclockwise;
}

#define GlowColor(color, colorIncrement, speed)                         \
{                                                                       \
    if (sprite->data[2] == 0)                                           \
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;          \
                                                                        \
    if (sprite->data[2] > 128)                                          \
    {                                                                   \
        BlendPalette(sprite->data[7], 16, 0, (color));                  \
        sprite->callback = WaitAnimEnd;                                 \
    }                                                                   \
    else                                                                \
    {                                                                   \
        sprite->data[6] = Sin(sprite->data[2], (colorIncrement));       \
        BlendPalette(sprite->data[7], 16, sprite->data[6], (color));    \
    }                                                                   \
    sprite->data[2] += (speed);                                         \
}

static void Anim_GlowBlack(struct Sprite *sprite)
{
    GlowColor(RGB_BLACK, 16, 1);
}

static void Anim_HorizontalStretch(struct Sprite *sprite)
{
    s16 index1 = 0, index2 = 0;

    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > 40)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        index2 = (sprite->data[2] * 128) / 40;

        if (sprite->data[2] >= 10 && sprite->data[2] <= 29)
        {
            sprite->data[7] += 51;
            index1 = 0xFF & sprite->data[7];
        }

        if (!sprite->sDontFlip)
            sprite->data[4] = (Sin(index2, 40) - 256) + Sin(index1, 16);
        else
            sprite->data[4] = (256 - Sin(index2, 40)) - Sin(index1, 16);

        sprite->data[5] = Sin(index2, 16) + 256;
        SetAffineData(sprite, sprite->data[4], sprite->data[5], 0);
    }

    sprite->data[2]++;
}

static void Anim_VerticalStretch(struct Sprite *sprite)
{
    s16 posY = 0, index1 = 0, index2 = 0;

    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > 40)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
        sprite->y2 = posY;
    }
    else
    {
        index2 = (sprite->data[2] * 128) / 40;

        if (sprite->data[2] >= 10 && sprite->data[2] <= 29)
        {
            sprite->data[7] += 51;
            index1 = 0xFF & sprite->data[7];
        }

        if (!sprite->sDontFlip)
            sprite->data[4] = -(Sin(index2, 16)) - 256;
        else
            sprite->data[4] = Sin(index2, 16) + 256;

        sprite->data[5] = (256 - Sin(index2, 40)) - Sin(index1, 8);

        if (sprite->data[5] != 256)
            posY = (256 - sprite->data[5]) / 8;

        sprite->y2 = -(posY);
        SetAffineData(sprite, sprite->data[4], sprite->data[5], 0);
    }

    sprite->data[2]++;
}

static void VerticalShakeTwice(struct Sprite *sprite)
{
    u8 index = sprite->data[2];
    u8 var7 = sprite->data[6];
    u8 var5 = sVerticalShakeData[sprite->data[5]][0];
    u8 var6 = sVerticalShakeData[sprite->data[5]][1];
    u8 amplitude = 0;

    if (var5 != (u8)-2)
        amplitude = (var6 - var7) * var5 / var6;
    else
        amplitude = 0;

    if (var5 == (u8)-1)
    {
        sprite->callback = WaitAnimEnd;
        sprite->y2 = 0;
    }
    else
    {
        sprite->y2 = Sin(index, amplitude);

        if (var7 == var6)
        {
            sprite->data[5]++;
            sprite->data[6] = 0;
        }
        else
        {
            sprite->data[2] += sprite->data[0];
            sprite->data[6]++;
        }
    }
}

static void Anim_VerticalShakeTwice(struct Sprite *sprite)
{
    sprite->data[0] = 48;
    VerticalShakeTwice(sprite);
    sprite->callback = VerticalShakeTwice;
}

static void Anim_TipMoveForward(struct Sprite *sprite)
{
    u8 counter = 0;

    TryFlipX(sprite);
    counter = sprite->data[2];

    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > 35)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
    }
    else
    {
        s16 index = ((counter - 10) * 128) / 20;

        if (counter < 10)
            HandleSetAffineData(sprite, 256, 256, counter / 2 * 512);
        else if (counter >= 10 && counter <= 29)
            sprite->x2 = -(Sin(index, 5));
        else
            HandleSetAffineData(sprite, 256, 256, (35 - counter) / 2 * 1024);
    }

    sprite->data[2]++;
    TryFlipX(sprite);
}

static void Anim_HorizontalPivot(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > 100)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->y2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 index = (sprite->data[2] * 256) / 100;
        sprite->y2 = Sin(index, 10);
        HandleSetAffineData(sprite, 256, 256, Sin(index, 3276));
    }

    sprite->data[2]++;
}

static void VerticalSlideWobble(struct Sprite *sprite)
{
    s32 var = 0;
    s16 index = 0;

    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > 100)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->y2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        index = (sprite->data[2] * 256) / 100;
        var = (sprite->data[2] * 512) / 100;
        var &= 0xFF;
        sprite->y2 = Sin(index, sprite->data[0]);
        HandleSetAffineData(sprite, 256, 256, Sin(var, 3276));
    }

    sprite->data[2]++;
}

static void Anim_VerticalSlideWobble(struct Sprite *sprite)
{
    sprite->data[0] = 10;
    VerticalSlideWobble(sprite);
    sprite->callback = VerticalSlideWobble;
}

static void RisingWobble(struct Sprite *sprite)
{
    s32 var = 0;
    s16 index = 0;

    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > 100)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->y2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        index = (sprite->data[2] * 256) / 100;
        var = (sprite->data[2] * 512) / 100;
        var &= 0xFF;
        sprite->y2 = -(Sin(index / 2, sprite->data[0] * 2));
        HandleSetAffineData(sprite, 256, 256, Sin(var, 3276));
    }

    sprite->data[2]++;
}

static void Anim_RisingWobble(struct Sprite *sprite)
{
    sprite->data[0] = 5;
    RisingWobble(sprite);
    sprite->callback = RisingWobble;
}

static void Anim_HorizontalSlideWobble(struct Sprite *sprite)
{
    s32 var;
    s16 index = 0;

    TryFlipX(sprite);
    var = 0;

    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    if (sprite->data[2] > 100)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->x2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        index = (sprite->data[2] * 256) / 100;
        var = (sprite->data[2] * 512) / 100;
        var &= 0xFF;
        sprite->x2 = Sin(index, 8);
        HandleSetAffineData(sprite, 256, 256, Sin(var, 3276));
    }

    sprite->data[2]++;
    TryFlipX(sprite);
}

static void VerticalSquishBounce(struct Sprite *sprite)
{
    s16 posY = 0;

    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[3] = 0;
    }

    TryFlipX(sprite);

    if (sprite->data[2] > sprite->data[0] * 3)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->y2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 yScale = Sin(sprite->data[4], 32) + 256;

        if (sprite->data[2] > sprite->data[0] && sprite->data[2] < sprite->data[0] * 2)
            sprite->data[3] += (128 / sprite->data[0]);
        if (yScale > 256)
            posY = (256 - yScale) / 8;

        sprite->y2 = -(Sin(sprite->data[3], 10)) - posY;
        HandleSetAffineData(sprite, 256 - Sin(sprite->data[4], 32), yScale, 0);
        sprite->data[2]++;
        sprite->data[4] = (sprite->data[4] + 128 / sprite->data[0]) & 0xFF;
    }

    TryFlipX(sprite);
}

static void Anim_VerticalSquishBounce(struct Sprite *sprite)
{
    sprite->data[0] = 16;
    VerticalSquishBounce(sprite);
    sprite->callback = VerticalSquishBounce;
}

static void ShrinkGrow(struct Sprite *sprite)
{
    s16 posY = 0;

    if (sprite->data[2] > (128 / sprite->data[6]) * sprite->data[7])
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->y2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 yScale = Sin(sprite->data[4], 32) + 256;

        if (yScale > 256)
            posY = (256 - yScale) / 8;

        sprite->y2 = -(posY);
        HandleSetAffineData(sprite, Sin(sprite->data[4], 48) + 256, yScale, 0);
        sprite->data[2]++;
        sprite->data[4] = (sprite->data[4] + sprite->data[6]) & 0xFF;
    }
}

static void Anim_ShrinkGrow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[7] = 3;
        sprite->data[6] = 8;
    }

    ShrinkGrow(sprite);
}

static const s8 sBounceRotateToSidesData[][8][3] =
{
    {
        { 0,  8,  8},
        { 8, -8, 12},
        {-8,  8, 12},
        { 8, -8, 12},
        {-8,  8, 12},
        { 8, -8, 12},
        {-8,  0, 12},
        { 0,  0,  0}
    },
    {
        { 0,  8, 16},
        { 8, -8, 24},
        {-8,  8, 24},
        { 8, -8, 24},
        {-8,  8, 24},
        { 8, -8, 24},
        {-8,  0, 24},
        { 0,  0,  0}
    },
};

static void BounceRotateToSides(struct Sprite *sprite)
{
    s16 var;
    u8 structId;
    s8 r9;
    s16 r10;
    s16 r7;
    u32 arrId;

    TryFlipX(sprite);
    structId = sprite->data[0];
    var = sAnims[structId].rotation;
    r9 = sBounceRotateToSidesData[sAnims[structId].data][sprite->data[4]][0];
    r10 = sBounceRotateToSidesData[sAnims[structId].data][sprite->data[4]][1] - r9;
    arrId = sAnims[structId].data;
    r7 = sprite->data[3];

    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
    }

    if (sBounceRotateToSidesData[arrId][sprite->data[4]][2] == 0)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->x2 = 0;
        sprite->y2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        u16 rotation;

        sprite->y2 = -(Sin(r7 * 128 / sBounceRotateToSidesData[arrId][sprite->data[4]][2], 10));
        sprite->x2 = (r10 * r7 / sBounceRotateToSidesData[arrId][sprite->data[4]][2]) + r9;

        rotation = -(var * sprite->x2) / 8;
        HandleSetAffineData(sprite, 256, 256, rotation);

        if (r7 == sBounceRotateToSidesData[arrId][sprite->data[4]][2])
        {
            sprite->data[4]++;
            sprite->data[3] = 0;
        }
        else
        {
            sprite->data[3]++;
        }
    }

    TryFlipX(sprite);
}

static void Anim_BounceRotateToSides(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();
    sAnims[id].rotation = 4096;
    sAnims[id].data = sprite->data[6];
    BounceRotateToSides(sprite);
    sprite->callback = BounceRotateToSides;
}

static void Anim_GlowOrange(struct Sprite *sprite)
{
    GlowColor(RGB(31, 22, 0), 12, 2);
}

static void Anim_GlowRed(struct Sprite *sprite)
{
    GlowColor(RGB_RED, 12, 2);
}

static void Anim_GlowBlue(struct Sprite *sprite)
{
    GlowColor(RGB_BLUE, 12, 2);
}

static void Anim_GlowYellow(struct Sprite *sprite)
{
    GlowColor(RGB_YELLOW, 12, 2);
}

static void Anim_GlowPurple(struct Sprite *sprite)
{
    GlowColor(RGB_PURPLE, 12, 2);
}

static void BackAndLunge_0(struct Sprite *sprite);
static void BackAndLunge_1(struct Sprite *sprite);
static void BackAndLunge_2(struct Sprite *sprite);
static void BackAndLunge_3(struct Sprite *sprite);
static void BackAndLunge_4(struct Sprite *sprite);

static void Anim_BackAndLunge(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->callback = BackAndLunge_0;
}

static void BackAndLunge_0(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (++sprite->x2 > 7)
    {
        sprite->x2 = 8;
        sprite->data[7] = 2;
        sprite->callback = BackAndLunge_1;
    }
    TryFlipX(sprite);
}

static void BackAndLunge_1(struct Sprite *sprite)
{
    TryFlipX(sprite);

    sprite->x2 -= sprite->data[7];
    sprite->data[7]++;
    if (sprite->x2 <= 0)
    {
        s16 subResult;
        u8 var = sprite->data[7];
        sprite->data[6] = 0;
        subResult = sprite->x2;

        do
        {
            subResult -= var;
            sprite->data[6]++;
            var++;
        }
        while (subResult > -8);

        sprite->data[5] = 1;
        sprite->callback = BackAndLunge_2;
    }

    TryFlipX(sprite);
}

static void BackAndLunge_2(struct Sprite *sprite)
{
    u8 rotation;

    TryFlipX(sprite);
    sprite->x2 -= sprite->data[7];
    sprite->data[7]++;
    rotation = (sprite->data[5] * 6) / sprite->data[6];

    if (++sprite->data[5] > sprite->data[6])
        sprite->data[5] = sprite->data[6];

    HandleSetAffineData(sprite, 256, 256, rotation * 256);

    if (sprite->x2 < -8)
    {
        sprite->x2 = -8;
        sprite->data[4] = 2;
        sprite->data[3] = 0;
        sprite->data[2] = rotation;
        sprite->callback = BackAndLunge_3;
    }

    TryFlipX(sprite);
}

static void BackAndLunge_3(struct Sprite *sprite)
{
    TryFlipX(sprite);

    if (sprite->data[3] > 11)
    {
        sprite->data[2] -= 2;
        if (sprite->data[2] < 0)
            sprite->data[2] = 0;

        HandleSetAffineData(sprite, 256, 256, sprite->data[2] << 8);
        if (sprite->data[2] == 0)
            sprite->callback = BackAndLunge_4;
    }
    else
    {
        sprite->x2 += sprite->data[4];
        sprite->data[4] *= -1;
        sprite->data[3]++;
    }

    TryFlipX(sprite);
}

static void BackAndLunge_4(struct Sprite *sprite)
{
    TryFlipX(sprite);

    sprite->x2 += 2;
    if (sprite->x2 > 0)
    {
        sprite->x2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }

    TryFlipX(sprite);
}

static void BackFlip_0(struct Sprite *sprite);
static void BackFlip_1(struct Sprite *sprite);
static void BackFlip_2(struct Sprite *sprite);

static void Anim_BackFlip(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->data[3] = 0;
    sprite->callback = BackFlip_0;
}

static void BackFlip_0(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2++;
    sprite->y2--;

    if (sprite->x2 % 2 == 0 && sprite->data[3] <= 0)
        sprite->data[3] = 10;
    if (sprite->x2 > 7)
    {
        sprite->x2 = 8;
        sprite->y2 = -8;
        sprite->data[4] = 0;
        sprite->callback = BackFlip_1;
    }

    TryFlipX(sprite);
}

static void BackFlip_1(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2 = Cos(sprite->data[4], 16) - 8;
    sprite->y2 = Sin(sprite->data[4], 16) - 8;

    if (sprite->data[4] > 63)
    {
        sprite->data[2] = 160;
        sprite->data[3] = 10;
        sprite->callback = BackFlip_2;
    }
    sprite->data[4] += 8;
    if (sprite->data[4] > 64)
        sprite->data[4] = 64;

    TryFlipX(sprite);
}

static void BackFlip_2(struct Sprite *sprite)
{
    TryFlipX(sprite);

    if (sprite->data[3] > 0)
    {
        sprite->data[3]--;
    }
    else
    {
        u32 rotation;

        sprite->x2 = Cos(sprite->data[2], 5) - 4;
        sprite->y2 = -(Sin(sprite->data[2], 5)) + 4;
        sprite->data[2] -= 4;
        rotation = sprite->data[2] - 32;
        HandleSetAffineData(sprite, 256, 256, rotation * 512);

        if (sprite->data[2] <= 32)
        {
            sprite->x2 = 0;
            sprite->y2 = 0;
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
        }
    }

    TryFlipX(sprite);
}

static void Anim_Flicker(struct Sprite *sprite)
{
    if (sprite->data[3] > 0)
    {
        sprite->data[3]--;
    }
    else
    {
        sprite->data[4] = (sprite->data[4] == 0) ? TRUE : FALSE;
        sprite->invisible = sprite->data[4];
        if (++sprite->data[2] > 19)
        {
            sprite->invisible = FALSE;
            sprite->callback = WaitAnimEnd;
        }
        sprite->data[3] = 2;
    }
}

static void BackFlipBig_0(struct Sprite *sprite);
static void BackFlipBig_1(struct Sprite *sprite);
static void BackFlipBig_2(struct Sprite *sprite);

static void Anim_BackFlipBig(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->callback = BackFlipBig_0;
}

static void BackFlipBig_0(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2--;
    sprite->y2++;

    if (sprite->x2 <= -16)
    {
        sprite->x2 = -16;
        sprite->y2 = 16;
        sprite->callback = BackFlipBig_1;
        sprite->data[2] = 160;
    }

    TryFlipX(sprite);
}

static void BackFlipBig_1(struct Sprite *sprite)
{
    u32 rotation;

    TryFlipX(sprite);
    sprite->data[2] -= 4;
    sprite->x2 = Cos(sprite->data[2], 22);
    sprite->y2 = -(Sin(sprite->data[2], 22));
    rotation = sprite->data[2] - 32;
    HandleSetAffineData(sprite, 256, 256, rotation * 512);

    if (sprite->data[2] <= 32)
        sprite->callback = BackFlipBig_2;

    TryFlipX(sprite);
}

static void BackFlipBig_2(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2--;
    sprite->y2++;

    if (sprite->x2 <= 0)
    {
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }

    TryFlipX(sprite);
}

static void FrontFlip_0(struct Sprite *sprite);
static void FrontFlip_1(struct Sprite *sprite);
static void FrontFlip_2(struct Sprite *sprite);

static void Anim_FrontFlip(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->callback = FrontFlip_0;
}

static void FrontFlip_0(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2++;
    sprite->y2--;

    if (sprite->x2 > 15)
    {
        sprite->data[2] = 0;
        sprite->callback = FrontFlip_1;
    }

    TryFlipX(sprite);
}

static void FrontFlip_1(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->data[2] += 16;

    if (sprite->x2 <= -16)
    {
        sprite->x2 = -16;
        sprite->y2 = 16;
        sprite->data[2] = 0;
        sprite->callback = FrontFlip_2;
    }
    else
    {
        sprite->x2 -= 2;
        sprite->y2 += 2;
    }

    HandleSetAffineData(sprite, 256, 256, sprite->data[2] << 8);
    TryFlipX(sprite);
}

static void FrontFlip_2(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2++;
    sprite->y2--;;

    if (sprite->x2 >= 0)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }

    TryFlipX(sprite);
}

static void TumblingFrontFlip(struct Sprite *sprite);

static void Anim_TumblingFrontFlip(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();
    sAnims[id].speed = 2;
    TumblingFrontFlip(sprite);
    sprite->callback = TumblingFrontFlip;
}

static void TumblingFrontFlip(struct Sprite *sprite)
{
    if (sAnims[sprite->data[0]].delay != 0)
    {
        sAnims[sprite->data[0]].delay--;
    }
    else
    {
        TryFlipX(sprite);
        if (sprite->data[2] == 0)
        {
            sprite->data[2]++;
            HandleStartAffineAnim(sprite);
            sprite->data[7] = sAnims[sprite->data[0]].speed;
            sprite->data[3] = -1;
            sprite->data[4] = -1;
            sprite->data[5] = 0;
            sprite->data[6] = 0;
        }

        sprite->x2 += (sprite->data[7] * 2 * sprite->data[3]);
        sprite->y2 += (sprite->data[7] * sprite->data[4]);
        sprite->data[6] += 8;
        if (sprite->x2 <= -16 || sprite->x2 >= 16)
        {
            sprite->x2 = sprite->data[3] * 16;
            sprite->data[3] *= -1;
            sprite->data[5]++;
        }
        else if (sprite->y2 <= -16 || sprite->y2 >= 16)
        {
            sprite->y2 = sprite->data[4] * 16;
            sprite->data[4] *= -1;
            sprite->data[5]++;
        }

        if (sprite->data[5] > 5 && sprite->x2 <= 0)
        {
            sprite->x2 = 0;
            sprite->y2 = 0;
            if (sAnims[sprite->data[0]].runs > 1)
            {
                sAnims[sprite->data[0]].runs--;
                sprite->data[5] = 0;
                sprite->data[6] = 0;
                sAnims[sprite->data[0]].delay = 10;
            }
            else
            {
                ResetSpriteAfterAnim(sprite);
                sprite->callback = WaitAnimEnd;
            }
        }

        HandleSetAffineData(sprite, 256, 256, sprite->data[6] << 8);
        TryFlipX(sprite);
    }
}

static void Figure8(struct Sprite *sprite);

static void Anim_Figure8(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->data[6] = 0;
    sprite->data[7] = 0;
    sprite->callback = Figure8;
}

static void Figure8(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->data[6] += 4;
    sprite->x2 = -(Sin(sprite->data[6], 16));
    sprite->y2 = -(Sin((sprite->data[6] * 2) & 0xFF, 8));
    if (sprite->data[6] > 192 && sprite->data[7] == 1)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->data[7]++;
    }
    else if (sprite->data[6] > 64 && sprite->data[7] == 0)
    {
        HandleSetAffineData(sprite, -256, 256, 0);
        sprite->data[7]++;
    }

    if (sprite->data[6] > 255)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    TryFlipX(sprite);
}

static void Anim_FlashYellow(struct Sprite *sprite)
{
    if (++sprite->data[2] == 1)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[6] = 0;
        sprite->data[5] = 0;
        sprite->data[4] = 0;
    }

    if (sYellowFlashData[sprite->data[6]][1] == (u8)-1)
    {
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        if (sprite->data[4] == 1)
        {
            if (sYellowFlashData[sprite->data[6]][0])
                BlendPalette(sprite->data[7], 16, 16, RGB_YELLOW);
            else
                BlendPalette(sprite->data[7], 16, 0, RGB_YELLOW);

            sprite->data[4] = 0;
        }

        if (sYellowFlashData[sprite->data[6]][1] == sprite->data[5])
        {
            sprite->data[4] = 1;
            sprite->data[5] = 0;
            sprite->data[6]++;
        }
        else
        {
            sprite->data[5]++;
        }
    }
}

static void SwingConcave(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    TryFlipX(sprite);
    if (sprite->data[2] > sAnims[sprite->data[0]].data)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->x2 = 0;
        if (sAnims[sprite->data[0]].runs > 1)
        {
            sAnims[sprite->data[0]].runs--;
            sprite->data[2] = 0;
        }
        else
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
        }
    }
    else
    {
        s16 index = (sprite->data[2] * 256) / sAnims[sprite->data[0]].data;
        sprite->x2 = -(Sin(index, 10));
        HandleSetAffineData(sprite, 256, 256, Sin(index, 3276));
    }

    sprite->data[2]++;
    TryFlipX(sprite);
}

static void Anim_SwingConcave_FastShort(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();
    sAnims[id].data = 50;
    SwingConcave(sprite);
    sprite->callback = SwingConcave;
}

static void SwingConvex(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
        HandleStartAffineAnim(sprite);

    TryFlipX(sprite);
    if (sprite->data[2] > sAnims[sprite->data[0]].data)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->x2 = 0;
        if (sAnims[sprite->data[0]].runs > 1)
        {
            sAnims[sprite->data[0]].runs--;
            sprite->data[2] = 0;
        }
        else
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
        }
    }
    else
    {
        s16 index = (sprite->data[2] * 256) / sAnims[sprite->data[0]].data;
        sprite->x2 = -(Sin(index, 10));
        HandleSetAffineData(sprite, 256, 256, -(Sin(index, 3276)));
    }

    sprite->data[2]++;
    TryFlipX(sprite);
}

static void Anim_SwingConvex_FastShort(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();
    sAnims[id].data = 50;
    SwingConvex(sprite);
    sprite->callback = SwingConvex;
}

static void RotateUpSlamDown_0(struct Sprite *sprite);
static void RotateUpSlamDown_1(struct Sprite *sprite);
static void RotateUpSlamDown_2(struct Sprite *sprite);

static void Anim_RotateUpSlamDown(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->data[6] = -(14 * sprite->centerToCornerVecX / 10);
    sprite->data[7] = 128;
    sprite->callback = RotateUpSlamDown_0;
}

static void RotateUpSlamDown_0(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->data[7]--;
    sprite->x2 = sprite->data[6] + Cos(sprite->data[7], sprite->data[6]);

    sprite->y2 = -(Sin(sprite->data[7], sprite->data[6]));

    HandleSetAffineData(sprite, 256, 256, (sprite->data[7] - 128) << 8);
    if (sprite->data[7] <= 120)
    {
        sprite->data[7] = 120;
        sprite->data[3] = 0;
        sprite->callback = RotateUpSlamDown_1;
    }

    TryFlipX(sprite);
}

static void RotateUpSlamDown_1(struct Sprite *sprite)
{
    if (sprite->data[3] == 20)
    {
        sprite->callback = RotateUpSlamDown_2;
        sprite->data[3] = 0;
    }

    sprite->data[3]++;
}

static void RotateUpSlamDown_2(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->data[7] += 2;
    sprite->x2 = sprite->data[6] + Cos(sprite->data[7], sprite->data[6]);

    sprite->y2 = -(Sin(sprite->data[7], sprite->data[6]));

    HandleSetAffineData(sprite, 256, 256, (sprite->data[7] - 128) << 8);
    if (sprite->data[7] >= 128)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->data[2] = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = Anim_VerticalShake;
    }

    TryFlipX(sprite);
}

static void DeepVerticalSquishBounce(struct Sprite *sprite)
{
    if (sAnims[sprite->data[0]].delay != 0)
    {
        sAnims[sprite->data[0]].delay--;
    }
    else
    {
        if (sprite->data[2] == 0)
        {
            HandleStartAffineAnim(sprite);
            sprite->data[4] = 0;
            sprite->data[5] = 0;
            sprite->data[2] = 1;
        }

        if (sprite->data[5] == 0)
        {
            sprite->data[7] = Sin(sprite->data[4], 256);
            sprite->y2 = Sin(sprite->data[4], 16);
            sprite->data[6] = Sin(sprite->data[4], 32);
            HandleSetAffineData(sprite, 256 - sprite->data[6], 256 + sprite->data[7], 0);
            if (sprite->data[4] == 128)
            {
                sprite->data[4] = 0;
                sprite->data[5] = 1;
            }
        }
        else if (sprite->data[5] == 1)
        {
            sprite->data[7] = Sin(sprite->data[4], 32);
            sprite->y2 = -(Sin(sprite->data[4], 8));
            sprite->data[6] = Sin(sprite->data[4], 128);
            HandleSetAffineData(sprite, 256 + sprite->data[6], 256 - sprite->data[7], 0);
            if (sprite->data[4] == 128)
            {
                if (sAnims[sprite->data[0]].runs > 1)
                {
                    sAnims[sprite->data[0]].runs--;
                    sAnims[sprite->data[0]].delay = 10;
                    sprite->data[4] = 0;
                    sprite->data[5] = 0;
                }
                else
                {
                    HandleSetAffineData(sprite, 256, 256, 0);
                    ResetSpriteAfterAnim(sprite);
                    sprite->callback = WaitAnimEnd;
                }
            }
        }

        sprite->data[4] += sAnims[sprite->data[0]].rotation;
    }
}

static void Anim_DeepVerticalSquishBounce(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();
    sAnims[id].rotation = 4;
    DeepVerticalSquishBounce(sprite);
    sprite->callback = DeepVerticalSquishBounce;
}

static void Anim_HorizontalJumps(struct Sprite *sprite)
{
    s32 counter = sprite->data[2];
    TryFlipX(sprite);
    if (counter > 512)
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
        sprite->y2 = 0;
    }
    else
    {
        switch (sprite->data[2] / 128)
        {
        case 0:
            sprite->x2 = -(counter % 128 * 8) / 128;
            break;
        case 1:
            sprite->x2 = (counter % 128 / 16) - 8;
            break;
        case 2:
            sprite->x2 = (counter % 128 / 16);
            break;
        case 3:
            sprite->x2 = -(counter % 128 * 8) / 128 + 8;
            break;
        }

        sprite->y2 = -(Sin(counter % 128, 8));
    }

    sprite->data[2] += 12;
    TryFlipX(sprite);
}

static void HorizontalJumpsVerticalStretch_0(struct Sprite *sprite);
static void HorizontalJumpsVerticalStretch_1(struct Sprite *sprite);
static void HorizontalJumpsVerticalStretch_2(struct Sprite *sprite);

static void Anim_HorizontalJumpsVerticalStretch(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();
    sAnims[id].data = -1;
    HandleStartAffineAnim(sprite);
    sprite->data[3] = 0;
    HorizontalJumpsVerticalStretch_0(sprite);
    sprite->callback = HorizontalJumpsVerticalStretch_0;
}

static void HorizontalJumpsVerticalStretch_0(struct Sprite *sprite)
{
    if (sAnims[sprite->data[0]].delay != 0)
    {
        sAnims[sprite->data[0]].delay--;
    }
    else
    {
        s32 counter;

        TryFlipX(sprite);
        counter = sprite->data[2];
        if (sprite->data[2] > 128)
        {
            sprite->data[2] = 0;
            sprite->callback = HorizontalJumpsVerticalStretch_1;
        }
        else
        {
            s32 var = 8 * sAnims[sprite->data[0]].data;
            sprite->x2 = var * (counter % 128) / 128;
            sprite->y2 = -(Sin(counter % 128, 8));
            sprite->data[2] += 12;
        }

        TryFlipX(sprite);
    }
}

static void HorizontalJumpsVerticalStretch_1(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] > 48)
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->y2 = 0;
        sprite->data[2] = 0;
        sprite->callback = HorizontalJumpsVerticalStretch_2;
    }
    else
    {
        s16 yDelta;
        s16 yScale = Sin(sprite->data[4], 64) + 256;
        if (sprite->data[2] >= 16 && sprite->data[2] <= 31)
        {
            sprite->data[3] += 8;
            sprite->x2 -= sAnims[sprite->data[0]].data;
        }

        yDelta = 0;
        if (yScale > 256)
            yDelta = (256 - yScale) / 8;

        sprite->y2 = -(Sin(sprite->data[3], 20)) - yDelta;
        HandleSetAffineData(sprite, 256 - Sin(sprite->data[4], 32), yScale, 0);
        sprite->data[2]++;
        sprite->data[4] += 8;
        sprite->data[4] &= 0xFF;
    }

    TryFlipX(sprite);
}

static void HorizontalJumpsVerticalStretch_2(struct Sprite *sprite)
{
    s32 counter;

    TryFlipX(sprite);
    counter = sprite->data[2];
    if (counter > 128)
    {
        if (sAnims[sprite->data[0]].runs > 1)
        {
            sAnims[sprite->data[0]].runs--;
            sAnims[sprite->data[0]].delay = 10;
            sprite->data[3] = 0;
            sprite->data[2] = 0;
            sprite->data[4] = 0;
            sprite->callback = HorizontalJumpsVerticalStretch_0;
        }
        else
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
        }

        sprite->x2 = 0;
        sprite->y2 = 0;
    }
    else
    {
        s32 var = sAnims[sprite->data[0]].data;

        sprite->x2 = var * ((counter % 128) * 8) / 128 + 8 * -var;
        sprite->y2 = -(Sin(counter % 128, 8));
    }

    sprite->data[2] += 12;
    TryFlipX(sprite);
}

static void RotateToSides(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
    }

    TryFlipX(sprite);
    if (sprite->data[7] > 254)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        if (sAnims[sprite->data[0]].runs > 1)
        {
            sAnims[sprite->data[0]].runs--;
            sprite->data[2] = 0;
            sprite->data[7] = 0;
        }
        else
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
        }

        TryFlipX(sprite);
    }
    else
    {
        u16 rotation;

        sprite->x2 = -(Sin(sprite->data[7], 16));
        rotation = Sin(sprite->data[7], 32);
        HandleSetAffineData(sprite, 256, 256, rotation << 8);
        sprite->data[7] += sAnims[sprite->data[0]].rotation;
        TryFlipX(sprite);
    }
}

static void Anim_RotateToSides_Fast(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();
    sAnims[id].rotation = 4;
    RotateToSides(sprite);
    sprite->callback = RotateToSides;
}

static void Anim_RotateUpToSides(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
    }

    TryFlipX(sprite);
    if (sprite->data[7] > 254)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
        TryFlipX(sprite);
    }
    else
    {
        u16 rotation;

        sprite->x2 = -(Sin(sprite->data[7], 16));
        sprite->y2 = -(Sin(sprite->data[7] % 128, 16));
        rotation = Sin(sprite->data[7], 32);
        HandleSetAffineData(sprite, 256, 256, rotation << 8);
        sprite->data[7] += 8;
        TryFlipX(sprite);
    }
}

static void Anim_FlickerIncreasing(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
        sprite->data[7] = 0;

    if (sprite->data[2] == sprite->data[7])
    {
        sprite->data[7] = 0;
        sprite->data[2]++;
        sprite->invisible = FALSE;
    }
    else
    {
        sprite->data[7]++;
        sprite->invisible = TRUE;
    }

    if (sprite->data[2] > 10)
    {
        sprite->invisible = FALSE;
        sprite->callback = WaitAnimEnd;
    }
}

static void TipHopForward_0(struct Sprite *sprite);
static void TipHopForward_1(struct Sprite *sprite);
static void TipHopForward_2(struct Sprite *sprite);

static void Anim_TipHopForward(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->data[7] = 0;
    sprite->callback = TipHopForward_0;
}

static void TipHopForward_0(struct Sprite *sprite)
{
    if (sprite->data[7] > 31)
    {
        sprite->data[7] = 32;
        sprite->data[2] = 0;
        sprite->callback = TipHopForward_1;
    }
    else
    {
        sprite->data[7] += 4;
    }

    HandleSetAffineData(sprite, 256, 256, sprite->data[7] << 8);
}

static void TipHopForward_1(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] > 512)
    {
        sprite->callback = TipHopForward_2;
        sprite->data[6] = 0;
    }
    else
    {
        sprite->x2 = -(sprite->data[2] * 16) / 512;
        sprite->y2 = -(Sin(sprite->data[2] % 128, 4));
        sprite->data[2] += 12;
    }

    TryFlipX(sprite);
}

static void TipHopForward_2(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->data[7] -= 2;
    if (sprite->data[7] < 0)
    {
        sprite->data[7] = 0;
        sprite->x2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        sprite->x2 = -(Sin(sprite->data[7] * 2, 16));
    }

    HandleSetAffineData(sprite, 256, 256, sprite->data[7] << 8);
    TryFlipX(sprite);
}

static void Anim_PivotShake(struct Sprite *sprite)
{
    u16 rotation;

    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
    }

    TryFlipX(sprite);
    if (sprite->data[7] > 255)
    {
        sprite->x2 = 0;
        sprite->y2 = 0;
        sprite->data[7] = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        sprite->data[7] += 16;
        sprite->x2 = -(Sin(sprite->data[7] % 128, 8));
        sprite->y2 = -(Sin(sprite->data[7] % 128, 8));
    }

    rotation = Sin(sprite->data[7] % 128, 16);
    HandleSetAffineData(sprite, 256, 256, rotation << 8);
    TryFlipX(sprite);
}

static void TipAndShake_0(struct Sprite *sprite);
static void TipAndShake_1(struct Sprite *sprite);
static void TipAndShake_2(struct Sprite *sprite);
static void TipAndShake_3(struct Sprite *sprite);

static void Anim_TipAndShake(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->data[7] = 0;
    sprite->data[4] = 0;
    sprite->callback = TipAndShake_0;
}

static void TipAndShake_0(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[7] > 24)
    {
        if (++sprite->data[4] > 4)
        {
            sprite->data[4] = 0;
            sprite->callback = TipAndShake_1;
        }
    }
    else
    {
        sprite->data[7] += 2;
        sprite->x2 = Sin(sprite->data[7], 8);
        sprite->y2 = -(Sin(sprite->data[7], 8));
    }

    HandleSetAffineData(sprite, 256, 256, -(sprite->data[7]) << 8);
    TryFlipX(sprite);
}

static void TipAndShake_1(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[7] > 32)
    {
        sprite->data[6] = 1;
        sprite->callback = TipAndShake_2;
    }
    else
    {
        sprite->data[7] += 2;
        sprite->x2 = Sin(sprite->data[7], 8);
        sprite->y2 = -(Sin(sprite->data[7], 8));
    }

    HandleSetAffineData(sprite, 256, 256, -(sprite->data[7]) << 8);
    TryFlipX(sprite);
}

static void TipAndShake_2(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->data[7] += (sprite->data[6] * 4);
    if (sprite->data[5] > 9)
    {
        sprite->data[7] = 32;
        sprite->callback = TipAndShake_3;
    }

    sprite->x2 = Sin(sprite->data[7], 8);
    sprite->y2 = -(Sin(sprite->data[7], 8));
    if (sprite->data[7] <= 28 || sprite->data[7] >= 36)
    {
        sprite->data[6] *= -1;
        sprite->data[5]++;
    }

    HandleSetAffineData(sprite, 256, 256, -(sprite->data[7]) << 8);
    TryFlipX(sprite);
}

static void TipAndShake_3(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[7] <= 0)
    {
        sprite->data[7] = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        sprite->data[7] -= 2;
        sprite->x2 = Sin(sprite->data[7], 8);
        sprite->y2 = -(Sin(sprite->data[7], 8));
    }

    HandleSetAffineData(sprite, 256, 256, -(sprite->data[7]) << 8);
    TryFlipX(sprite);
}

static void Anim_VibrateToCorners(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] > 40)
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
    }
    else
    {
        s8 sign;
        if (!(sprite->data[2] & 1))
            sign = 1;
        else
            sign = -1;

        if ((sprite->data[2] % 4) / 2 == 0)
        {
            sprite->x2 = Sin((sprite->data[2] * 128 / 40) % 256, 16) * sign;
            sprite->y2 = -(sprite->x2);
        }
        else
        {
            sprite->x2 = -(Sin((sprite->data[2] * 128 / 40) % 256, 16)) * sign;
            sprite->y2 = sprite->x2;
        }
    }

    sprite->data[2]++;
    TryFlipX(sprite);
}

static void Anim_GrowInStages(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[5] = 0;
        sprite->data[6] = 0;
        sprite->data[7] = 0;
        sprite->data[2]++;
    }

    if (sprite->data[6] > 0)
    {
        sprite->data[6]--;
        if (sprite->data[5] != 3)
        {
            s16 scale = (8 * sprite->data[6]) / 20;
            scale = Sin(sprite->data[7] - scale, 64);
            HandleSetAffineData(sprite, 256 - scale, 256 - scale, 0);
        }
    }
    else
    {
        s16 var;

        if (sprite->data[5] == 3)
        {
            if (sprite->data[7] > 63)
            {
                sprite->data[7] = 64;
                HandleSetAffineData(sprite, 256, 256, 0);
                ResetSpriteAfterAnim(sprite);
                sprite->callback = WaitAnimEnd;
            }
            var = Cos(sprite->data[7], 64);
        }
        else
        {
            var = Sin(sprite->data[7], 64);
            if (sprite->data[7] > 63)
            {
                sprite->data[5] = 3;
                sprite->data[6] = 10;
                sprite->data[7] = 0;
            }
            else
            {
                if (var > 48 && sprite->data[5] == 1)
                {
                    sprite->data[5] = 2;
                    sprite->data[6] = 20;
                }
                else if (var > 16 && sprite->data[5] == 0)
                {
                    sprite->data[5] = 1;
                    sprite->data[6] = 20;
                }
            }
        }

        sprite->data[7] += 2;
        HandleSetAffineData(sprite, 256 - var, 256 - var, 0);
    }

    TryFlipX(sprite);
}

static void Anim_VerticalSpring(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
    }

    if (sprite->data[7] > 512)
    {
        sprite->y2 = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 yScale;

        sprite->y2 = Sin(sprite->data[7] % 256, 8);
        sprite->data[7] += 8;
        yScale = Sin(sprite->data[7] % 128, 96);
        HandleSetAffineData(sprite, 256, yScale + 256, 0);
    }
}

static void Anim_VerticalRepeatedSpring(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
    }

    if (sprite->data[7] > 256)
    {
        sprite->y2 = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 yScale;

        sprite->y2 = Sin(sprite->data[7], 16);
        sprite->data[7] += 4;
        yScale = Sin((sprite->data[7] % 64) * 2, 128);
        HandleSetAffineData(sprite, 256, yScale + 256, 0);
    }
}

static void SpringRising_0(struct Sprite *sprite);
static void SpringRising_1(struct Sprite *sprite);
static void SpringRising_2(struct Sprite *sprite);

static void Anim_SpringRising(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->callback = SpringRising_0;
    sprite->data[7] = 0;
}

static void SpringRising_0(struct Sprite *sprite)
{
    s16 yScale;

    sprite->data[7] += 8;
    if (sprite->data[7] > 63)
    {
        sprite->data[7] = 0;
        sprite->data[6] = 0;
        sprite->callback = SpringRising_1;
        yScale = Sin(64, 128); // 128 * 1 = 128
    }
    else
    {
        yScale = Sin(sprite->data[7], 128);
    }

    HandleSetAffineData(sprite, 256, 256 + yScale, 0);
}

static void SpringRising_1(struct Sprite *sprite)
{
    s16 yScale;

    sprite->data[7] += 4;
    if (sprite->data[7] > 95)
    {
        yScale = Cos(0, 128); // 128 * (-1) = -128
        sprite->data[7] = 0;
        sprite->data[6]++;
    }
    else
    {
        s16 sign, index;

        sprite->y2 = -(sprite->data[6] * 4) - Sin(sprite->data[7], 8);
        if (sprite->data[7] > 63)
        {
            sign = -1;
            index = sprite->data[7] - 64;
        }
        else
        {
            sign = 1;
            index = 0;
        }

        yScale = Cos((index * 2) + sprite->data[7], 128) * sign;
    }

    HandleSetAffineData(sprite, 256, 256 + yScale, 0);
    if (sprite->data[6] == 3)
    {
        sprite->data[7] = 0;
        sprite->callback = SpringRising_2;
    }
}

static void SpringRising_2(struct Sprite *sprite)
{
    s16 yScale;

    sprite->data[7] += 8;
    yScale = Cos(sprite->data[7], 128);
    sprite->y2 = -(Cos(sprite->data[7], 12));
    if (sprite->data[7] > 63)
    {
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
        sprite->y2 = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
    }

    HandleSetAffineData(sprite, 256, 256 + yScale, 0);
}

static void HorizontalSpring(struct Sprite *sprite)
{
    if (sprite->data[7] > sprite->data[5])
    {
        sprite->x2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
        HandleSetAffineData(sprite, 256, 256, 0);
    }
    else
    {
        s16 xScale;

        sprite->x2 = Sin(sprite->data[7] % 256, sprite->data[4]);
        sprite->data[7] += sprite->data[6];
        xScale = Sin(sprite->data[7] % 128, 96);
        HandleSetAffineData(sprite, 256 + xScale, 256, 0);
    }
}

static void Anim_HorizontalSpring(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
        sprite->data[6] = 8;
        sprite->data[5] = 512;
        sprite->data[4] = 8;
    }

    HorizontalSpring(sprite);
}

static void HorizontalRepeatedSpring(struct Sprite *sprite)
{
    if (sprite->data[7] > sprite->data[5])
    {
        sprite->x2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
        HandleSetAffineData(sprite, 256, 256, 0);
    }
    else
    {
        s16 xScale;

        sprite->x2 = Sin(sprite->data[7] % 256, sprite->data[4]);
        sprite->data[7] += sprite->data[6];
        xScale = Sin((sprite->data[7] % 64) * 2, 128);
        HandleSetAffineData(sprite, 256 + xScale, 256, 0);
    }
}

static void Anim_HorizontalRepeatedSpring_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
        sprite->data[6] = 4;
        sprite->data[5] = 256;
        sprite->data[4] = 16;
    }

    HorizontalRepeatedSpring(sprite);
}

static void Anim_HorizontalSlideShrink(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
    }

    if (sprite->data[7] > 512)
    {
        sprite->x2 = 0;
        ResetSpriteAfterAnim(sprite);
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 scale;

        sprite->x2 = Sin(sprite->data[7] % 256, 8);
        sprite->data[7] += 8;
        scale = Sin(sprite->data[7] % 128, 96);
        HandleSetAffineData(sprite, 256 + scale, 256 + scale, 0);
    }

    TryFlipX(sprite);
}

static void Anim_LungeGrow(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
    }

    if (sprite->data[7] > 512)
    {
        sprite->x2 = 0;
        ResetSpriteAfterAnim(sprite);
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 scale;

        sprite->x2 = -(Sin((sprite->data[7] % 256) / 2, 16));
        sprite->data[7] += 8;
        scale = -(Sin((sprite->data[7] % 256) / 2, 64));
        HandleSetAffineData(sprite, 256 + scale, 256 + scale, 0);
    }

    TryFlipX(sprite);
}

static void Anim_CircleIntoBackground(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
    }

    if (sprite->data[7] > 512)
    {
        sprite->x2 = 0;
        ResetSpriteAfterAnim(sprite);
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        s16 scale;

        sprite->x2 = -(Sin(sprite->data[7] % 256 , 8));
        sprite->data[7] += 8;
        scale = Sin((sprite->data[7] % 256) / 2, 96);
        HandleSetAffineData(sprite, 256 + scale, 256 + scale, 0);
    }

    TryFlipX(sprite);
}

static void Anim_RapidHorizontalHops(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] > 2048)
    {
        sprite->callback = WaitAnimEnd;
        sprite->data[6] = 0;
    }
    else
    {
        s16 caseVar = (sprite->data[2] / 512) % 4;
        switch (caseVar)
        {
        case 0:
            sprite->x2 = -(sprite->data[2] % 512 * 16) / 512;
            break;
        case 1:
            sprite->x2 = (sprite->data[2] % 512 / 32) - 16;
            break;
        case 2:
            sprite->x2 = (sprite->data[2] % 512) / 32;
            break;
        case 3:
            sprite->x2 = -(sprite->data[2] % 512 * 16) / 512 + 16;
            break;
        }

        sprite->y2 = -(Sin(sprite->data[2] % 128, 4));
        sprite->data[2] += 24;
    }

    TryFlipX(sprite);
}

static void Anim_FourPetal(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] == 0)
    {
        sprite->data[6] = 0;
        sprite->data[7] = 64;
        sprite->data[2]++;
    }

    sprite->data[7] += 8;
    if (sprite->data[6] == 4)
    {
        if (sprite->data[7] > 63)
        {
            sprite->data[7] = 0;
            sprite->data[6]++;
        }
    }
    else
    {
        if (sprite->data[7] > 127)
        {
            sprite->data[7] = 0;
            sprite->data[6]++;
        }
    }

    switch (sprite->data[6])
    {
    case 1:
        sprite->x2 = -(Cos(sprite->data[7], 8));
        sprite->y2 = Sin(sprite->data[7], 8) - 8;
        break;
    case 2:
        sprite->x2 = Sin(sprite->data[7] + 128, 8) + 8;
        sprite->y2 = -(Cos(sprite->data[7], 8));
        break;
    case 3:
        sprite->x2 = Cos(sprite->data[7], 8);
        sprite->y2 = Sin(sprite->data[7] + 128, 8) + 8;
        break;
    case 0:
    case 4:
        sprite->x2 = Sin(sprite->data[7], 8) - 8;
        sprite->y2 = Cos(sprite->data[7], 8);
        break;
    default:
        sprite->x2 = 0;
        sprite->y2 = 0;
        sprite->callback = WaitAnimEnd;
        break;
    }

    TryFlipX(sprite);
}

static void Anim_VerticalSquishBounce_Slow(struct Sprite *sprite)
{
    sprite->data[0] = 32;
    VerticalSquishBounce(sprite);
    sprite->callback = VerticalSquishBounce;
}

static void Anim_HorizontalSlide_Slow(struct Sprite *sprite)
{
    sprite->data[0] = 80;
    HorizontalSlide(sprite);
    sprite->callback = HorizontalSlide;
}

static void Anim_VerticalSlide_Slow(struct Sprite *sprite)
{
    sprite->data[0] = 80;
    VerticalSlide(sprite);
    sprite->callback = VerticalSlide;
}

static void Anim_BounceRotateToSides_Small(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].rotation = 2048;
    sAnims[id].data = sprite->data[6];
    BounceRotateToSides(sprite);
    sprite->callback = BounceRotateToSides;
}

static void Anim_BounceRotateToSides_Slow(struct Sprite *sprite)
{
    sprite->data[6] = 1;
    Anim_BounceRotateToSides(sprite);
}

static void Anim_BounceRotateToSides_SmallSlow(struct Sprite *sprite)
{
    sprite->data[6] = 1;
    Anim_BounceRotateToSides_Small(sprite);
}

static void Anim_ZigzagSlow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
        sprite->data[0] = 0;

    if (sprite->data[0] <= 0)
    {
        Zigzag(sprite);
        sprite->data[0] = 1;
    }
    else
    {
        sprite->data[0]--;
    }
}

static void Anim_HorizontalShake_Slow(struct Sprite *sprite)
{
    sprite->data[0] = 30;
    sprite->data[7] = 3;
    HorizontalShake(sprite);
    sprite->callback = HorizontalShake;
}

static void Anim_VertialShake_Slow(struct Sprite *sprite)
{
    sprite->data[0] = 30;
    VerticalShake(sprite);
    sprite->callback = VerticalShake;
}

static void Anim_Twist_Twice(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].rotation = 1024;
    sAnims[id].delay = 0;
    sAnims[id].runs = 2;
    Twist(sprite);
    sprite->callback = Twist;
}

static void Anim_CircleCounterclockwise_Slow(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].rotation = 512;
    sAnims[id].data = 3;
    sAnims[id].speed = 12;
    CircleCounterclockwise(sprite);
    sprite->callback = CircleCounterclockwise;
}

static void Anim_VerticalShakeTwice_Slow(struct Sprite *sprite)
{
    sprite->data[0] = 24;
    VerticalShakeTwice(sprite);
    sprite->callback = VerticalShakeTwice;
}

static void Anim_VerticalSlideWobble_Small(struct Sprite *sprite)
{
    sprite->data[0] = 5;
    VerticalSlideWobble(sprite);
    sprite->callback = VerticalSlideWobble;
}

static void Anim_VerticalJumps_Small(struct Sprite *sprite)
{
    sprite->data[0] = 3;
    VerticalJumps(sprite);
    sprite->callback = VerticalJumps;
}

static void Anim_Spin(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].delay = 60;
    sAnims[id].data = 30;
    Spin(sprite);
    sprite->callback = Spin;
}

static void Anim_TumblingFrontFlip_Twice(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].speed = 1;
    sAnims[id].runs = 2;
    TumblingFrontFlip(sprite);
    sprite->callback = TumblingFrontFlip;
}

static void Anim_DeepVerticalSquishBounce_Twice(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].rotation = 4;
    sAnims[id].runs = 2;
    DeepVerticalSquishBounce(sprite);
    sprite->callback = DeepVerticalSquishBounce;
}

static void Anim_HorizontalJumpsVerticalStretch_Twice(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].data = 1;
    sAnims[id].runs = 2;
    HandleStartAffineAnim(sprite);
    sprite->data[3] = 0;
    HorizontalJumpsVerticalStretch_0(sprite);
    sprite->callback = HorizontalJumpsVerticalStretch_0;
}

static void Anim_RotateToSides(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].rotation = 2;
    RotateToSides(sprite);
    sprite->callback = RotateToSides;
}

static void Anim_RotateToSides_Twice(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].rotation = 4;
    sAnims[id].runs = 2;
    RotateToSides(sprite);
    sprite->callback = RotateToSides;
}

static void Anim_SwingConcave(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].data = 100;
    SwingConcave(sprite);
    sprite->callback = SwingConcave;
}

static void Anim_SwingConcave_Fast(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].data = 50;
    sAnims[id].runs = 2;
    SwingConcave(sprite);
    sprite->callback = SwingConcave;
}

static void Anim_SwingConvex(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].data = 100;
    SwingConvex(sprite);
    sprite->callback = SwingConvex;
}

static void Anim_SwingConvex_Fast(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].data = 50;
    sAnims[id].runs = 2;
    SwingConvex(sprite);
    sprite->callback = SwingConvex;
}

// Very similar to VerticalShake, used by back animations only
static void VerticalShakeBack(struct Sprite *sprite)
{
    s32 counter = sprite->data[2];
    if (counter > 2304)
    {
        sprite->callback = WaitAnimEnd;
        sprite->y2 = 0;
    }
    else
    {
        sprite->y2 = Sin((counter + 192) % 256, sprite->data[7]) + sprite->data[7];
    }

    sprite->data[2] += sprite->data[0];
}

static void Anim_VerticalShakeBack(struct Sprite *sprite)
{
    sprite->data[0] = 60;
    sprite->data[7] = 3;
    VerticalShakeBack(sprite);
    sprite->callback = VerticalShakeBack;
}

static void Anim_VerticalShakeBack_Slow(struct Sprite *sprite)
{
    sprite->data[0] = 30;
    sprite->data[7] = 3;
    VerticalShakeBack(sprite);
    sprite->callback = VerticalShakeBack;
}

static void Anim_VerticalShakeHorizontalSlide_Slow(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] > 2048)
    {
        sprite->callback = WaitAnimEnd;
        sprite->data[6] = 0;
    }
    else
    {
        s16 divCase = (sprite->data[2] / 512) % 4;
        switch (divCase)
        {
        case 0:
            sprite->x2 = (sprite->data[2] % 512) / 32;
            break;
        case 2:
            sprite->x2 = -(sprite->data[2] % 512 * 16) / 512;
            break;
        case 1:
            sprite->x2 = -(sprite->data[2] % 512 * 16) / 512 + 16;
            break;
        case 3:
            sprite->x2 = (sprite->data[2] % 512) / 32 - 16;
            break;
        }

        sprite->y2 = Sin(sprite->data[2] % 128, 4);
        sprite->data[2] += 24;
    }

    TryFlipX(sprite);
}

static void VerticalStretchBothEnds(struct Sprite *sprite)
{
    s16 index1 = 0, index2 = 0;

    if (sprite->data[5] > sprite->data[6])
    {
        sprite->y2 = 0;
        sprite->data[5] = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        if (sprite->data[4] <= 1)
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
        }
        else
        {
            sprite->data[4]--;
            sprite->data[7] = 0;
        }
    }
    else
    {
        u8 amplitude, cmpVal1, cmpVal2;
        s16 xScale, yScale;

        index2 = (sprite->data[5] * 128) / sprite->data[6];
        cmpVal1 = sprite->data[6] / 4;
        cmpVal2 = cmpVal1 * 3;
        if (sprite->data[5] >= cmpVal1 && sprite->data[5] < cmpVal2)
        {
            sprite->data[7] += 51;
            index1 = sprite->data[7] & 0xFF;
        }

        if (!sprite->sDontFlip)
            xScale = -256 - Sin(index2, 16);
        else
            xScale = 256 + Sin(index2, 16);

        amplitude = sprite->data[3];
        yScale = 256 - Sin(index2, amplitude) - Sin(index1, amplitude / 5);
        SetAffineData(sprite, xScale, yScale, 0);
        sprite->data[5]++;
    }
}

static void Anim_VerticalStretchBothEnds_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        HandleStartAffineAnim(sprite);
        sprite->data[4] = 1;
        sprite->data[6] = 40;
        sprite->data[3] = 40;
        sprite->data[5] = 0;
        sprite->data[7] = 0;
    }

    VerticalStretchBothEnds(sprite);
}

static void HorizontalStretchFar(struct Sprite *sprite)
{
    s16 index1 = 0, index2;

    if (sprite->data[5] > sprite->data[6])
    {
        sprite->data[5] = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        if (sprite->data[4] <= 1)
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
        }
        else
        {
            sprite->data[4]--;
            sprite->data[7] = 0;
        }
    }
    else
    {
        u8 amplitude, cmpVal1, cmpVal2;
        s16 xScale;

        index2 = (sprite->data[5] * 128) / sprite->data[6];
        cmpVal1 = sprite->data[6] / 4;
        cmpVal2 = cmpVal1 * 3;
        if (sprite->data[5] >= cmpVal1 && sprite->data[5] < cmpVal2)
        {
            sprite->data[7] += 51;
            index1 = sprite->data[7] & 0xFF;
        }

        amplitude = sprite->data[3];

        if (!sprite->sDontFlip)
            xScale = -256 + Sin(index2, amplitude) + Sin(index1, amplitude / 5 * 2);
        else
            xScale = 256 - Sin(index2, amplitude) - Sin(index1, amplitude / 5 * 2);

        SetAffineData(sprite, xScale, 256, 0);
        sprite->data[5]++;
    }
}

static void Anim_HorizontalStretchFar_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        HandleStartAffineAnim(sprite);
        sprite->data[4] = 1;
        sprite->data[6] = 40;
        sprite->data[3] = 40;
        sprite->data[5] = 0;
        sprite->data[7] = 0;
    }

    HorizontalStretchFar(sprite);
}

static void VerticalShakeLowTwice(struct Sprite *sprite)
{
    u8 var6, var7;
    u8 var8 = sprite->data[2];
    u8 var9 = sprite->data[6];
    u8 var5 = sVerticalShakeData[sprite->data[5]][0];
    if (var5 != (u8)-1)
        var5 = sprite->data[7];

    var6 = sVerticalShakeData[sprite->data[5]][1];
    var7 = 0;
    if (sVerticalShakeData[sprite->data[5]][0] != (u8)-2)
        var7 = (var6 - var9) * var5 / var6;
    else
        var7 = 0;

    if (var5 == (u8)-1)
    {
        sprite->callback = WaitAnimEnd;
        sprite->y2 = 0;
    }
    else
    {
        sprite->y2 = Sin((var8 + 192) % 256, var7) + var7;
        if (var9 == var6)
        {
            sprite->data[5]++;
            sprite->data[6] = 0;
        }
        else
        {
            sprite->data[2] += sprite->data[0];
            sprite->data[6]++;
        }
    }
}

// Very similar in appearance to Anim_VerticalShakeTwice (especially the fast variant), but deeper
static void Anim_VerticalShakeLowTwice(struct Sprite *sprite)
{
    sprite->data[0] = 40;
    sprite->data[7] = 6;
    VerticalShakeLowTwice(sprite);
    sprite->callback = VerticalShakeLowTwice;
}

static void Anim_HorizontalShake_Fast(struct Sprite *sprite)
{
    sprite->data[0] = 70;
    sprite->data[7] = 6;
    HorizontalShake(sprite);
    sprite->callback = HorizontalShake;
}

static void Anim_HorizontalSlide_Fast(struct Sprite *sprite)
{
    sprite->data[0] = 20;
    HorizontalSlide(sprite);
    sprite->callback = HorizontalSlide;
}

static void Anim_HorizontalVibrate_Fast(struct Sprite *sprite)
{
    if (sprite->data[2] > 40)
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
    }
    else
    {
        s8 sign;
        if (!(sprite->data[2] & 1))
            sign = 1;
        else
            sign = -1;

        sprite->x2 = Sin((sprite->data[2] * 128 / 40) % 256, 9) * sign;
    }

    sprite->data[2]++;
}

static void Anim_HorizontalVibrate_Fastest(struct Sprite *sprite)
{
    if (sprite->data[2] > 40)
    {
        sprite->callback = WaitAnimEnd;
        sprite->x2 = 0;
    }
    else
    {
        s8 sign;
        if (!(sprite->data[2] & 1))
            sign = 1;
        else
            sign = -1;

        sprite->x2 = Sin((sprite->data[2] * 128 / 40) % 256, 12) * sign;
    }

    sprite->data[2]++;
}

static void Anim_VerticalShakeBack_Fast(struct Sprite *sprite)
{
    sprite->data[0] = 70;
    sprite->data[7] = 6;
    VerticalShakeBack(sprite);
    sprite->callback = VerticalShakeBack;
}

static void Anim_VerticalShakeLowTwice_Slow(struct Sprite *sprite)
{
    sprite->data[0] = 24;
    sprite->data[7] = 6;
    VerticalShakeLowTwice(sprite);
    sprite->callback = VerticalShakeLowTwice;
}

static void Anim_VerticalShakeLowTwice_Fast(struct Sprite *sprite)
{
    sprite->data[0] = 56;
    sprite->data[7] = 9;
    VerticalShakeLowTwice(sprite);
    sprite->callback = VerticalShakeLowTwice;
}

static void Anim_CircleCounterclockwise_Long(struct Sprite *sprite)
{
    u8 id = sprite->data[0] = AddNewAnim();

    sAnims[id].rotation = 1024;
    sAnims[id].data = 6;
    sAnims[id].speed = 24;
    CircleCounterclockwise(sprite);
    sprite->callback = CircleCounterclockwise;
}

static void GrowStutter(struct Sprite *sprite)
{
    s16 index1 = 0, index2 = 0;
    if (sprite->data[5] > sprite->data[6])
    {
        sprite->y2 = 0;
        sprite->data[5] = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        if (sprite->data[4] <= 1)
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
        }
        else
        {
            sprite->data[4]--;
            sprite->data[7] = 0;
        }
    }
    else
    {
        u8 amplitude, cmpVal1, cmpVal2;
        s16 xScale, yScale;

        index2 = (sprite->data[5] * 128) / sprite->data[6];
        cmpVal1 = sprite->data[6] / 4;
        cmpVal2 = cmpVal1 * 3;
        if (sprite->data[5] >= cmpVal1 && sprite->data[5] < cmpVal2)
        {
            sprite->data[7] += 51;
            index1 = sprite->data[7] & 0xFF;
        }

        amplitude = sprite->data[3];

        if (!sprite->sDontFlip)
            xScale = Sin(index2, amplitude) + (Sin(index1, amplitude / 5 * 2) - 256);
        else
            xScale = 256 - Sin(index1, amplitude / 5 * 2) - Sin(index2, amplitude);

        yScale = 256 - Sin(index1, amplitude / 5) - Sin(index2, amplitude);
        SetAffineData(sprite, xScale, yScale, 0);
        sprite->data[5]++;
    }
}

static void Anim_GrowStutter_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        HandleStartAffineAnim(sprite);
        sprite->data[4] = 1;
        sprite->data[6] = 40;
        sprite->data[3] = 40;
        sprite->data[5] = 0;
        sprite->data[7] = 0;
    }

    GrowStutter(sprite);
}

static void Anim_VerticalShakeHorizontalSlide(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] > 2048)
    {
        sprite->callback = WaitAnimEnd;
        sprite->data[6] = 0;
    }
    else
    {
        s16 divCase = (sprite->data[2] / 512) % 4;
        switch (divCase)
        {
        case 0:
            sprite->x2 = (sprite->data[2] % 512) / 32;
            break;
        case 2:
            sprite->x2 = -(sprite->data[2] % 512 * 16) / 512;
            break;
        case 1:
            sprite->x2 = -(sprite->data[2] % 512 * 16) / 512 + 16;
            break;
        case 3:
            sprite->x2 = (sprite->data[2] % 512) / 32 - 16;
            break;
        }

        sprite->y2 = Sin(sprite->data[2] % 128, 4);
        sprite->data[2] += 48;
    }

    TryFlipX(sprite);
}

static void Anim_VerticalShakeHorizontalSlide_Fast(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] > 2048)
    {
        sprite->callback = WaitAnimEnd;
        sprite->data[6] = 0;
    }
    else
    {
        s16 divCase = (sprite->data[2] / 512) % 4;
        switch (divCase)
        {
        case 0:
            sprite->x2 = (sprite->data[2] % 512) / 32;
            break;
        case 2:
            sprite->x2 = -(sprite->data[2] % 512 * 16) / 512;
            break;
        case 1:
            sprite->x2 = -(sprite->data[2] % 512 * 16) / 512 + 16;
            break;
        case 3:
            sprite->x2 = (sprite->data[2] % 512) / 32 - 16;
            break;
        }

        sprite->y2 = Sin(sprite->data[2] % 96, 4);
        sprite->data[2] += 64;
    }

    TryFlipX(sprite);
}

static const s8 sTriangleDownData[][3] =
{
//   x    y   timer
    {1,   1, 12},
    {-2,  0, 12},
    {1,  -1, 12},
    {0,  0,  0}
};

static void TriangleDown(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] == 0)
        sprite->data[3] = 0;

    if (sTriangleDownData[sprite->data[3]][2] / sprite->data[5] == sprite->data[2])
    {
        sprite->data[3]++;
        sprite->data[2] = 0;
    }

    if (sTriangleDownData[sprite->data[3]][2] / sprite->data[5] == 0)
    {
        if (--sprite->data[6] == 0)
            sprite->callback = WaitAnimEnd;
        else
            sprite->data[2] = 0;
    }
    else
    {
        s32 amplitude = sprite->data[5];
        sprite->x2 += (sTriangleDownData[sprite->data[3]][0] * amplitude);
        sprite->y2 += (sTriangleDownData[sprite->data[3]][1] * sprite->data[5]); // Not using amplitude here. No reason for this.
        sprite->data[2]++;
        TryFlipX(sprite);
    }
}

static void Anim_TriangleDown_Slow(struct Sprite *sprite)
{
    sprite->data[5] = 1;
    sprite->data[6] = 1;
    TriangleDown(sprite);
    sprite->callback = TriangleDown;
}

static void Anim_TriangleDown(struct Sprite *sprite)
{
    sprite->data[5] = 2;
    sprite->data[6] = 1;
    TriangleDown(sprite);
    sprite->callback = TriangleDown;
}

static void Anim_TriangleDown_Fast(struct Sprite *sprite)
{
    sprite->data[5] = 2;
    sprite->data[6] = 2;
    TriangleDown(sprite);
    sprite->callback = TriangleDown;
}

static void Grow(struct Sprite *sprite)
{
    if (sprite->data[7] > 255)
    {
        if (sprite->data[5] <= 1)
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
            HandleSetAffineData(sprite, 256, 256, 0);
        }
        else
        {
            sprite->data[5]--;
            sprite->data[7] = 0;
        }
    }
    else
    {
        s16 scale;

        sprite->data[7] += sprite->data[6];
        if (sprite->data[7] > 256)
            sprite->data[7] = 256;

        scale = Sin(sprite->data[7] / 2, 64);
        HandleSetAffineData(sprite, 256 - scale, 256 - scale, 0);
    }
}

static void Anim_Grow(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
        sprite->data[6] = 4;
        sprite->data[5] = 1;
    }

    Grow(sprite);
    TryFlipX(sprite);
}

static void Anim_Grow_Twice(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
        sprite->data[6] = 8;
        sprite->data[5] = 2;
    }

    Grow(sprite);
    TryFlipX(sprite);
}

static void Anim_HorizontalSpring_Fast(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
        sprite->data[6] = 8;
        sprite->data[5] = 512;
        sprite->data[4] = 16;
    }

    HorizontalSpring(sprite);
}

static void Anim_HorizontalSpring_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
        sprite->data[6] = 4;
        sprite->data[5] = 256;
        sprite->data[4] = 16;
    }

    HorizontalSpring(sprite);
}

static void Anim_HorizontalRepeatedSpring_Fast(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
        sprite->data[6] = 8;
        sprite->data[5] = 512;
        sprite->data[4] = 16;
    }

    HorizontalRepeatedSpring(sprite);
}

static void Anim_HorizontalRepeatedSpring(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[2]++;
        sprite->data[7] = 0;
        sprite->data[6] = 8;
        sprite->data[5] = 512;
        sprite->data[4] = 8;
    }

    HorizontalRepeatedSpring(sprite);
}

static void Anim_ShrinkGrow_Fast(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[7] = 5;
        sprite->data[6] = 8;
    }

    ShrinkGrow(sprite);
}

static void Anim_ShrinkGrow_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[7] = 3;
        sprite->data[6] = 4;
    }

    ShrinkGrow(sprite);
}

static void Anim_VerticalStretchBothEnds(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        HandleStartAffineAnim(sprite);
        sprite->data[4] = 1;
        sprite->data[6] = 30;
        sprite->data[3] = 60;
        sprite->data[7] = 0;
    }

    VerticalStretchBothEnds(sprite);
}

static void Anim_VerticalStretchBothEnds_Twice(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        HandleStartAffineAnim(sprite);
        sprite->data[4] = 2;
        sprite->data[6] = 20;
        sprite->data[3] = 70;
        sprite->data[7] = 0;
    }

    VerticalStretchBothEnds(sprite);
}

static void Anim_HorizontalStretchFar_Twice(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        HandleStartAffineAnim(sprite);
        sprite->data[4] = 2;
        sprite->data[6] = 20;
        sprite->data[3] = 70;
        sprite->data[5] = 0;
        sprite->data[7] = 0;
    }

    HorizontalStretchFar(sprite);
}

static void Anim_HorizontalStretchFar(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        HandleStartAffineAnim(sprite);
        sprite->data[4] = 1;
        sprite->data[6] = 30;
        sprite->data[3] = 60;
        sprite->data[5] = 0;
        sprite->data[7] = 0;
    }

    HorizontalStretchFar(sprite);
}

static void Anim_GrowStutter_Twice(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        HandleStartAffineAnim(sprite);
        sprite->data[4] = 2;
        sprite->data[6] = 20;
        sprite->data[3] = 70;
        sprite->data[5] = 0;
        sprite->data[7] = 0;
    }

    GrowStutter(sprite);
}

static void Anim_GrowStutter(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        HandleStartAffineAnim(sprite);
        sprite->data[4] = 1;
        sprite->data[6] = 30;
        sprite->data[3] = 60;
        sprite->data[5] = 0;
        sprite->data[7] = 0;
    }

    GrowStutter(sprite);
}

static void ConcaveArc(struct Sprite *sprite)
{
    if (sprite->data[7] > 255)
    {
        if (sprite->data[6] <= 1)
        {
            sprite->callback = WaitAnimEnd;
            sprite->x2 = 0;
            sprite->y2 = 0;
        }
        else
        {
            sprite->data[7] %= 256;
            sprite->data[6]--;
        }
    }
    else
    {
        sprite->x2 = -(Sin(sprite->data[7], sprite->data[5]));
        sprite->y2 = Sin((sprite->data[7] + 192) % 256, sprite->data[4]);
        if (sprite->y2 > 0)
            sprite->y2 *= -1;

        sprite->y2 += sprite->data[4];
        sprite->data[7] += sprite->data[3];
    }
}

static void Anim_ConcaveArcLarge_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        sprite->data[6] = 1;
        sprite->data[7] = 0;
        sprite->data[5] = 12;
        sprite->data[4] = 12;
        sprite->data[3] = 4;
    }

    ConcaveArc(sprite);
}

static void Anim_ConcaveArcLarge(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        sprite->data[6] = 1;
        sprite->data[7] = 0;
        sprite->data[5] = 12;
        sprite->data[4] = 12;
        sprite->data[3] = 6;
    }

    ConcaveArc(sprite);
}

static void Anim_ConcaveArcLarge_Twice(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        sprite->data[6] = 2;
        sprite->data[7] = 0;
        sprite->data[5] = 12;
        sprite->data[4] = 12;
        sprite->data[3] = 8;
    }

    ConcaveArc(sprite);
}

static void ConvexDoubleArc(struct Sprite *sprite)
{
    if (sprite->data[7] > 256)
    {
        if (sprite->data[6] <= sprite->data[4])
        {
            sprite->callback = WaitAnimEnd;
        }
        else
        {
            sprite->data[4]++;
            sprite->data[7] = 0;
        }

        sprite->x2 = 0;
        sprite->y2 = 0;
    }
    else
    {
        s16 posX;

        if (sprite->data[7] > 159)
        {
            if (sprite->data[7] > 256)
                sprite->data[7] = 256;

            sprite->y2 = -(Sin(sprite->data[7] % 256, 8));
        }
        else if (sprite->data[7] > 95)
        {
            sprite->y2 = Sin(96, 6) - Sin((sprite->data[7] - 96) * 2, 4);
        }
        else
        {
            sprite->y2 = Sin(sprite->data[7], 6);
        }

        posX = -(Sin(sprite->data[7] / 2, sprite->data[5]));
        if (sprite->data[4] % 2 == 0)
            posX *= -1;

        sprite->x2 = posX;
        sprite->data[7] += sprite->data[3];
    }
}

static void Anim_ConvexDoubleArc_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        sprite->data[6] = 2;
        sprite->data[7] = 0;
        sprite->data[5] = 16;
        sprite->data[4] = 1;
        sprite->data[3] = 4;
    }

    ConvexDoubleArc(sprite);
}

static void Anim_ConvexDoubleArc(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        sprite->data[6] = 2;
        sprite->data[7] = 0;
        sprite->data[5] = 16;
        sprite->data[4] = 1;
        sprite->data[3] = 6;
    }

    ConvexDoubleArc(sprite);
}

static void Anim_ConvexDoubleArc_Twice(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        sprite->data[6] = 3;
        sprite->data[7] = 0;
        sprite->data[5] = 16;
        sprite->data[4] = 1;
        sprite->data[3] = 8;
    }

    ConvexDoubleArc(sprite);
}

static void Anim_ConcaveArcSmall_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        sprite->data[6] = 1;
        sprite->data[7] = 0;
        sprite->data[5] = 4;
        sprite->data[4] = 6;
        sprite->data[3] = 4;
    }

    ConcaveArc(sprite);
}

static void Anim_ConcaveArcSmall(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        sprite->data[6] = 1;
        sprite->data[7] = 0;
        sprite->data[5] = 4;
        sprite->data[4] = 6;
        sprite->data[3] = 6;
    }

    ConcaveArc(sprite);
}

static void Anim_ConcaveArcSmall_Twice(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[2] = 1;
        sprite->data[6] = 2;
        sprite->data[7] = 0;
        sprite->data[5] = 4;
        sprite->data[4] = 6;
        sprite->data[3] = 8;
    }

    ConcaveArc(sprite);
}

static void SetHorizontalDip(struct Sprite *sprite)
{
    u16 index = Sin((sprite->data[2] * 128) / sprite->data[7], sprite->data[5]);
    sprite->data[6] = -(index << 8);
    SetPosForRotation(sprite, index, sprite->data[4], 0);
    HandleSetAffineData(sprite, 256, 256, sprite->data[6]);
}

static void Anim_HorizontalDip(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[7] = 60;
        sprite->data[5] = 8;
        sprite->data[4] = -32;
        sprite->data[3] = 1;
        sprite->data[0] = 0;
    }

    if (sprite->data[2] > sprite->data[7])
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->x2 = 0;
        sprite->y2 = 0;
        sprite->data[0]++;
        if (sprite->data[3] <= sprite->data[0])
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
            return;
        }
        else
        {
            sprite->data[2] = 0;
        }
    }
    else
    {
        SetHorizontalDip(sprite);
    }

    sprite->data[2]++;
}

static void Anim_HorizontalDip_Fast(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[7] = 90;
        sprite->data[5] = 8;
        sprite->data[4] = -32;
        sprite->data[3] = 1;
        sprite->data[0] = 0;
    }

    if (sprite->data[2] > sprite->data[7])
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->x2 = 0;
        sprite->y2 = 0;
        sprite->data[0]++;
        if (sprite->data[3] <= sprite->data[0])
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
            return;
        }
        else
        {
            sprite->data[2] = 0;
        }
    }
    else
    {
        SetHorizontalDip(sprite);
    }

    sprite->data[2]++;
}

static void Anim_HorizontalDip_Twice(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->data[7] = 30;
        sprite->data[5] = 8;
        sprite->data[4] = -32;
        sprite->data[3] = 2;
        sprite->data[0] = 0;
    }

    if (sprite->data[2] > sprite->data[7])
    {
        HandleSetAffineData(sprite, 256, 256, 0);
        sprite->x2 = 0;
        sprite->y2 = 0;
        sprite->data[0]++;
        if (sprite->data[3] <= sprite->data[0])
        {
            ResetSpriteAfterAnim(sprite);
            sprite->callback = WaitAnimEnd;
            return;
        }
        else
        {
            sprite->data[2] = 0;
        }
    }
    else
    {
        SetHorizontalDip(sprite);
    }

    sprite->data[2]++;
}

static void ShrinkGrowVibrate(struct Sprite *sprite)
{
    if (sprite->data[2] > sprite->data[7])
    {
        sprite->y2 = 0;
        HandleSetAffineData(sprite, 256, 256, 0);
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        u8 posY_unsigned;
        s8 posY_signed;
        s32 posY;
        s16 index = (u16)(sprite->data[2] % sprite->data[6] * 256) / sprite->data[6] % 256;
        if (sprite->data[2] % 2 == 0)
        {
            sprite->data[4] = Sin(index, 32) + 256;
            sprite->data[5] = Sin(index, 32) + 256;
            posY_unsigned = Sin(index, 32);
            posY_signed = posY_unsigned;
        }
        else
        {
            sprite->data[4] = Sin(index, 8) + 256;
            sprite->data[5] = Sin(index, 8) + 256;
            posY_unsigned = Sin(index, 8);
            posY_signed = posY_unsigned;
        }

        posY = posY_signed;
        if (posY < 0)
            posY += 7;
        sprite->y2 = (u32)(posY) >> 3;
        HandleSetAffineData(sprite, sprite->data[4], sprite->data[5], 0);
    }

    sprite->data[2]++;
}

static void Anim_ShrinkGrowVibrate_Fast(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->y2 += 2;
        sprite->data[6] = 40;
        sprite->data[7] = 80;
    }

    ShrinkGrowVibrate(sprite);
}

static void Anim_ShrinkGrowVibrate(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->y2 += 2;
        sprite->data[6] = 40;
        sprite->data[7] = 40;
    }

    ShrinkGrowVibrate(sprite);
}

static void Anim_ShrinkGrowVibrate_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        HandleStartAffineAnim(sprite);
        sprite->y2 += 2;
        sprite->data[6] = 80;
        sprite->data[7] = 80;
    }

    ShrinkGrowVibrate(sprite);
}

static void JoltRight_0(struct Sprite *sprite);
static void JoltRight_1(struct Sprite *sprite);
static void JoltRight_2(struct Sprite *sprite);
static void JoltRight_3(struct Sprite *sprite);

static void JoltRight(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2 -= sprite->data[2];
    if (sprite->x2 <= -sprite->data[6])
    {
        sprite->x2 = -sprite->data[6];
        sprite->data[7] = 2;
        sprite->callback = JoltRight_0;
    }

    TryFlipX(sprite);
}

static void JoltRight_0(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2 += sprite->data[7];
    sprite->data[7]++;
    if (sprite->x2 >= 0)
        sprite->callback = JoltRight_1;

    TryFlipX(sprite);
}

static void JoltRight_1(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2 += sprite->data[7];
    sprite->data[7]++;
    if (sprite->x2 > sprite->data[6])
    {
        sprite->x2 = sprite->data[6];
        sprite->callback = JoltRight_2;
    }

    TryFlipX(sprite);
}

static void JoltRight_2(struct Sprite *sprite)
{
    TryFlipX(sprite);
    if (sprite->data[3] >= sprite->data[5])
    {
        sprite->callback = JoltRight_3;
    }
    else
    {
        sprite->x2 += sprite->data[4];
        sprite->data[4] *= -1;
        sprite->data[3]++;
    }

    TryFlipX(sprite);
}

static void JoltRight_3(struct Sprite *sprite)
{
    TryFlipX(sprite);
    sprite->x2 -= 2;
    if (sprite->x2 <= 0)
    {
        sprite->x2 = 0;
        ResetSpriteAfterAnim(sprite);
        sprite->callback = WaitAnimEnd;
    }

    TryFlipX(sprite);
}

static void Anim_JoltRight_Fast(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->data[7] = 4;
    sprite->data[6] = 12;
    sprite->data[5] = 16;
    sprite->data[4] = 4;
    sprite->data[3] = 0;
    sprite->data[2] = 2;
    sprite->callback = JoltRight;
}

static void Anim_JoltRight(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->data[7] = 2;
    sprite->data[6] = 8;
    sprite->data[5] = 12;
    sprite->data[4] = 2;
    sprite->data[3] = 0;
    sprite->data[2] = 1;
    sprite->callback = JoltRight;
}

static void Anim_JoltRight_Slow(struct Sprite *sprite)
{
    HandleStartAffineAnim(sprite);
    sprite->data[7] = 0;
    sprite->data[6] = 6;
    sprite->data[5] = 6;
    sprite->data[4] = 2;
    sprite->data[3] = 0;
    sprite->data[2] = 1;
    sprite->callback = JoltRight;
}

static void SetShakeFlashYellowPos(struct Sprite *sprite)
{
    sprite->x2 = sprite->data[1];
    if (sprite->data[0] > 1)
    {
        sprite->data[1] *= -1;
        sprite->data[0] = 0;
    }
    else
    {
        sprite->data[0]++;
    }
}

static const struct YellowFlashData sShakeYellowFlashData_Fast[] =
{
    {FALSE,  1},
    { TRUE,  2},
    {FALSE, 15},
    { TRUE,  1},
    {FALSE, 15},
    { TRUE,  1},
    {FALSE, 15},
    { TRUE,  1},
    {FALSE,  1},
    { TRUE,  1},
    {FALSE,  1},
    { TRUE,  1},
    {FALSE,  1},
    { TRUE,  1},
    {FALSE,  1},
    { TRUE,  1},
    {FALSE,  1},
    { TRUE,  1},
    {FALSE,  1},
    {FALSE, -1}
};

static const struct YellowFlashData sShakeYellowFlashData_Normal[] =
{
    {FALSE,  5},
    { TRUE,  1},
    {FALSE, 15},
    { TRUE,  4},
    {FALSE,  2},
    { TRUE,  2},
    {FALSE,  2},
    { TRUE,  2},
    {FALSE,  2},
    { TRUE,  2},
    {FALSE,  2},
    { TRUE,  2},
    {FALSE,  2},
    {FALSE, -1}
};

static const struct YellowFlashData sShakeYellowFlashData_Slow[] =
{
    {FALSE,  1},
    { TRUE,  1},
    {FALSE, 20},
    { TRUE,  1},
    {FALSE, 20},
    { TRUE,  1},
    {FALSE, 20},
    { TRUE,  1},
    {FALSE,  1},
    {FALSE, -1}
};

static const struct YellowFlashData *const sShakeYellowFlashData[] =
{
    sShakeYellowFlashData_Fast,
    sShakeYellowFlashData_Normal,
    sShakeYellowFlashData_Slow
};

static void ShakeFlashYellow(struct Sprite *sprite)
{
    const struct YellowFlashData *array = sShakeYellowFlashData[sprite->data[3]];
    SetShakeFlashYellowPos(sprite);
    if (array[sprite->data[6]].time == (u8)-1)
    {
        sprite->x2 = 0;
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        if (sprite->data[4] == 1)
        {
            if (array[sprite->data[6]].isYellow)
                BlendPalette(sprite->data[7], 16, 16, RGB_YELLOW);
            else
                BlendPalette(sprite->data[7], 16, 0, RGB_YELLOW);

            sprite->data[4] = 0;
        }

        if (array[sprite->data[6]].time == sprite->data[5])
        {
            sprite->data[4] = 1;
            sprite->data[5] = 0;
            sprite->data[6]++;
        }
        else
        {
            sprite->data[5]++;
        }
    }
}

static void Anim_ShakeFlashYellow_Fast(struct Sprite *sprite)
{
    if (++sprite->data[2] == 1)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[6] = 0;
        sprite->data[5] = 0;
        sprite->data[4] = 0;
        sprite->data[3] = 0;
    }

    ShakeFlashYellow(sprite);
}

static void Anim_ShakeFlashYellow(struct Sprite *sprite)
{
    if (++sprite->data[2] == 1)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[6] = 0;
        sprite->data[5] = 0;
        sprite->data[4] = 0;
        sprite->data[3] = 1;
    }

    ShakeFlashYellow(sprite);
}

static void Anim_ShakeFlashYellow_Slow(struct Sprite *sprite)
{
    if (++sprite->data[2] == 1)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[6] = 0;
        sprite->data[5] = 0;
        sprite->data[4] = 0;
        sprite->data[3] = 2;
    }

    ShakeFlashYellow(sprite);
}

enum {
    SHAKEGLOW_RED,
    SHAKEGLOW_GREEN,
    SHAKEGLOW_BLUE,
    SHAKEGLOW_BLACK,
    SHAKEGLOW_WHITE,
    SHAKEGLOW_PURPLE
};

static void ShakeGlow_Blend(struct Sprite *sprite)
{
    static const u16 sColors[] =
    {
        [SHAKEGLOW_RED]   = RGB_RED,
        [SHAKEGLOW_GREEN] = RGB_GREEN,
        [SHAKEGLOW_BLUE]  = RGB_BLUE,
        [SHAKEGLOW_BLACK] = RGB_BLACK,
        [SHAKEGLOW_WHITE] = RGB_WHITE,
        [SHAKEGLOW_PURPLE] = RGB_PURPLE
    };

    if (sprite->data[2] > 127)
    {
        BlendPalette(sprite->data[7], 16, 0, RGB_RED);
        sprite->callback = WaitAnimEnd;
    }
    else
    {
        sprite->data[6] = Sin(sprite->data[2], 12);
        BlendPalette(sprite->data[7], 16, sprite->data[6], sColors[sprite->data[1]]);
    }
}

static void ShakeGlow_Move(struct Sprite *sprite)
{
    if (sprite->data[3] < sprite->data[4])
    {
        TryFlipX(sprite);
        if (sprite->data[5] > sprite->data[0])
        {
            if (++sprite->data[3] < sprite->data[4])
                sprite->data[5] = 0;

            sprite->x2 = 0;
        }
        else
        {
            s8 sign = 1 - (sprite->data[3] % 2 * 2);
            sprite->x2 = sign * Sin((sprite->data[5] * 384 / sprite->data[0]) % 256, 6);
            sprite->data[5]++;
        }

        TryFlipX(sprite);
    }
}

static void Anim_ShakeGlowRed_Fast(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 10;
        sprite->data[5] = 0;
        sprite->data[4] = 2;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_RED;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowRed(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 20;
        sprite->data[5] = 0;
        sprite->data[4] = 1;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_RED;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowRed_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 80;
        sprite->data[5] = 0;
        sprite->data[4] = 1;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_RED;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowGreen_Fast(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 10;
        sprite->data[5] = 0;
        sprite->data[4] = 2;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_GREEN;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowGreen(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 20;
        sprite->data[5] = 0;
        sprite->data[4] = 1;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_GREEN;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowGreen_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 80;
        sprite->data[5] = 0;
        sprite->data[4] = 1;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_GREEN;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowBlue_Fast(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 10;
        sprite->data[5] = 0;
        sprite->data[4] = 2;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_BLUE;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowBlue(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 20;
        sprite->data[5] = 0;
        sprite->data[4] = 1;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_BLUE;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowBlue_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 80;
        sprite->data[5] = 0;
        sprite->data[4] = 1;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_BLUE;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowBlack_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 80;
        sprite->data[5] = 0;
        sprite->data[4] = 1;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_BLACK;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowWhite_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 80;
        sprite->data[5] = 0;
        sprite->data[4] = 1;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_WHITE;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void Anim_ShakeGlowPurple_Slow(struct Sprite *sprite)
{
    if (sprite->data[2] == 0)
    {
        sprite->data[7] = (sprite->oam.paletteNum * 16) + 256;
        sprite->data[0] = 80;
        sprite->data[5] = 0;
        sprite->data[4] = 1;
        sprite->data[3] = 0;
        sprite->data[1] = SHAKEGLOW_PURPLE;
    }

    if (sprite->data[2] % 2 == 0)
        ShakeGlow_Blend(sprite);

    if (sprite->data[2] >= (128 - sprite->data[0] * sprite->data[4]) / 2)
        ShakeGlow_Move(sprite);

    sprite->data[2]++;
}

static void WaitAnimEnd(struct Sprite *sprite)
{
    if (sprite->animEnded)
        sprite->callback = SpriteCallbackDummy;
}
