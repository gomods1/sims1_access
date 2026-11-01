// Every string the mod speaks, in English: the source the translations in lang/ follow.
// X(Id, "English"), grouped by feature. A translation keeps the printf placeholders (it may
// reorder them as %1$s, %2$d); tools/mod_strings.py checks that and lists missing translations.
// The direction words (Dir*) are only ever inserted into TilesAway, OneTileAway, FacingDirection,
// InCorner, AgainstWall, BesideWall, DoorInWall and StairsClimb; a translation may store them in the form those need
// (Finnish keeps compound stems), so never speak one on its own.
#pragma once

#define SA_STRINGS(X) \
    /* Startup */                                                                                                      \
    X(ModLoaded, "Sims accessibility loaded")                                                                          \
    /* Key help (Ctrl+F1) */                                                                                           \
    X(KeysScreen,                                                                                                      \
      "Tab and Shift+Tab go from group to group; the arrows move within a group, Home and End go to its ends; "        \
      "Enter presses, and on a kind or a room of the lot's things opens its things; Backspace goes back; R "           \
      "repeats. On a slider or value picker, left and right change it. Control+Up and Control+Down read the "          \
      "details of what was said, Control+Left and Control+Right switch between details and events.")                   \
    X(KeysLot,                                                                                                         \
      "Tab and Shift+Tab go through the groups: the status line, the needs, the family, the action queue, things "     \
      "by kind, things by room, then the game's panels; the arrows move within a group, and on a settings page "       \
      "left and right change a slider or value picker. Page Down and Page Up go through the lot's things, nearest "    \
      "first, and the camera goes to each one; Shift with them changes the kind of thing, Control with them the "      \
      "room. Enter on a thing opens its interactions, Control+Enter walks there. R repeats. Space selects the "        \
      "next Sim, Control+1 to 9 select a Sim of the family, Control+F makes the camera follow the selected Sim or "    \
      "not. Control+E says what the selected Sim is doing and needs, Control+N changes how much the narrator "         \
      "says. P pauses and reads the time and money. Control+Up and Control+Down read the details, Control+Left "       \
      "and Control+Right switch between details and events.")                                                          \
    X(KeysPlacing,                                                                                                     \
      "Page Down and Page Up go through the spots where it fits and can be used, best first; Control with them "       \
      "changes room; Shift with them, or comma and period, turn it on the same spot. Enter buys it there, Escape "     \
      "cancels, R repeats. Control+Up and Control+Down read the spot's details and what else was checked.")            \
    X(KeysBuy,                                                                                                         \
      "Arrows go through the catalog; a product reads its price and description, Control+Up and Control+Down "         \
      "everything about it. Enter on a product starts placing it where it fits and can be used. Page Down and "        \
      "Page Up go through the lot's things, Shift with them the kinds, Control with them the rooms; Enter on a "       \
      "thing picks it up to move it, Delete then sells it.")                                                           \
    X(KeysMenu, "Up and down choose an interaction, Enter does it, Escape goes back.")                                 \
    X(UnsupportedBuild, "Sims accessibility: unsupported game version, mod disabled")                                  \
    /* Control state */                                                                                                \
    X(Checked, "checked")                                                                                              \
    X(NotChecked, "not checked")                                                                                       \
    X(Selected, "selected")                                                                                            \
    X(TabSelected, "selected")                                                                                         \
    X(Unavailable, "unavailable")                                                                                      \
    X(UnlabeledButton, "unlabeled button")                                                                             \
    X(Unnamed, "unnamed")                                                                                              \
    /* Control roles: what a control is, said after its name */                                                        \
    X(RoleButton, "button")                                                                                            \
    X(RoleTab, "tab")                                                                                                  \
    X(RoleCheckBox, "check box")                                                                                       \
    X(RoleSlider, "slider")                                                                                            \
    X(RolePicker, "value picker")                                                                                      \
    X(NothingHere, "nothing to navigate here")                                                                         \
    X(NextPage, "next page")                                                                                           \
    X(BufferDetails, "details")                                                                                        \
    X(CheatConsole, "cheat console")                                                                                   \
    X(CheatsUnavailable, "the cheat console is not available here")                                                    \
    X(CheatRan, "cheat: %s")                                                                                           \
    X(CheatOf, "%s, %d of %d")                                                                                         \
    X(CheatEmpty, "type or pick a cheat first")                                                                        \
    X(CheatHint,                                                                                                       \
      "Up and Down pick a cheat, typing adds to it, Enter runs it and Enter again repeats it, Escape closes.")         \
    X(FundsNow, "funds %s, %s")                                                                                        \
    X(FundsChanged, "%1$s, %2$s left")                                                                                 \
    X(BufferEvents, "events")                                                                                          \
    X(BufferEmpty, "%s, empty")                                                                                        \
    X(PositionInGroup, "%d of %d, %s")                                                                                 \
    X(TileAt, "tile %d, %d")                                                                                           \
    X(SellsFor, "sells for %s")                                                                                        \
    X(PreviousPage, "previous page")                                                                                   \
    /* Groups */                                                                                                       \
    X(GroupSummary, "%s, %d items")                                                                                    \
    X(GroupSummaryOne, "%s, 1 item")                                                                                   \
    X(GroupButtons, "buttons")                                                                                         \
    X(GroupText, "text")                                                                                               \
    X(GroupHouses, "houses")                                                                                           \
    X(GroupFamilies, "families")                                                                                       \
    X(GroupControls, "controls")                                                                                       \
    X(GroupOptions, "options")                                                                                         \
    X(GroupCatalog, "catalog")                                                                                         \
    X(GroupBuild, "build")                                                                                             \
    X(GroupCamera, "camera")                                                                                           \
    X(GroupFamilyPanel, "Sim panels")                                                                                  \
    X(GroupActionQueue, "action queue")                                                                                \
    /* Screens */                                                                                                      \
    X(ScreenNeighborhood, "Neighborhood")                                                                              \
    X(ScreenNeighborhoodNumber, "Neighborhood %s")                                                                     \
    X(ScreenLot, "Lot")                                                                                                \
    X(ScreenPickFamily, "Choose a family")                                                                             \
    X(ScreenDesignFamily, "Create a family")                                                                           \
    X(ScreenDesignCharacter, "Create a Sim")                                                                           \
    /* Widgets */                                                                                                      \
    X(MotiveValue, "%s, %d percent")                                                                                   \
    X(QueuedAction, "%s, queued")                                                                                      \
    X(TutorialIcon, "help system tip")                                                                                 \
    X(HouseNumber, "House %d")                                                                                         \
    X(FamilyMemberOne, "1 member")                                                                                     \
    X(FamilyMembers, "%d members")                                                                                     \
    X(NetWorth, "net worth")                                                                                           \
    X(Friends, "friends")                                                                                              \
    /* Lot: the lot's things by kind and by room */                                                                        \
    X(CategorySims, "Sims")                                                                                            \
    X(CategorySeating, "seating")                                                                                      \
    X(CategorySurfaces, "surfaces")                                                                                    \
    X(CategoryAppliances, "appliances")                                                                                \
    X(CategoryElectronics, "electronics")                                                                              \
    X(CategoryPlumbing, "plumbing")                                                                                    \
    X(CategoryLighting, "lighting")                                                                                    \
    X(CategoryDecorative, "decorative")                                                                                \
    X(CategoryMiscellaneous, "miscellaneous")                                                                          \
    X(CategoryOther, "other objects")                                                                                  \
    X(ThingsByKind,                                                                                                    \
      "things by kind")                                                                                                \
    X(ThingsByRoom,                                                                                                    \
      "things by room")                                                                                                \
    X(NoObjects, "no objects here")                                                                                    \
    /* Lot: rooms */                                                                                                   \
    X(RoomOutside, "outside")                                                                                          \
    X(RoomBathroom, "bathroom")                                                                                        \
    X(RoomBedroom, "bedroom")                                                                                          \
    X(RoomKitchen, "kitchen")                                                                                          \
    X(RoomDining, "dining room")                                                                                       \
    X(RoomLiving, "living room")                                                                                       \
    X(RoomStudy, "study")                                                                                              \
    X(RoomOther, "room")                                                                                               \
    X(FloorNumber, "floor %d")                                                                                         \
    X(RoomObjects, "%s, %d objects")                                                                                   \
    X(RoomObjectsOne, "%s, 1 object")                                                                                  \
    X(RoomTiles, "%d tiles")                                                                                           \
    X(SameObjects, "%d alike")                                                                                         \
    X(JustBought, "just bought")                                                                                       \
    X(NotOnLot, "not on a lot")                                                                                        \
    X(HereNow, "right here")                                                                                           \
    X(TilesAway, "%d tiles %s")                                                                                        \
    X(OneTileAway, "1 tile %s")                                                                                        \
    X(DirNorth, "north")                                                                                               \
    X(DirNorthEast, "north-east")                                                                                      \
    X(DirEast, "east")                                                                                                 \
    X(DirSouthEast, "south-east")                                                                                      \
    X(DirSouth, "south")                                                                                               \
    X(DirSouthWest, "south-west")                                                                                      \
    X(DirWest, "west")                                                                                                 \
    X(DirNorthWest, "north-west")                                                                                      \
    X(OtherFloor, "on another floor")                                                                                  \
    X(SelectedSimSuffix, "selected")                                                                                   \
    X(PieMenu, "interactions")                                                                                         \
    X(NoInteractions, "no interactions")                                                                               \
    X(NoInteractionsAtAll, "no interactions: nothing can be done with this object")                                    \
    X(OnlyForChildren, "only for children: %s")                                                                        \
    X(OnlyForAdults, "only for adults: %s")                                                                            \
    X(NotAvailableNow, "not available right now: %s")                                                                  \
    X(Cancelled, "cancelled")                                                                                          \
    X(WalkingThere, "walking there")                                                                                   \
    X(CannotWalk, "cannot walk there")                                                                                 \
    /* Camera and Sim selection (nav/camera.h) */                                                                      \
    X(FollowOn, "following %s")                                                                                        \
    X(FollowOnNoSim, "follow on; no Sim selected")                                                                     \
    X(FollowOff, "camera free, not following")                                                                         \
    X(NoSuchSim, "no Sim %d")                                                                                          \
    X(SimSelectedNow, "%s, selected")                                                                                  \
    /* The tutorial and help tips (nav/tutorial.h) */                                                                  \
    X(GroupTutorial, "tutorial")                                                                                       \
    X(TutorialSays, "Tutorial:")                                                                                       \
    X(TutorialPointsAt, "the arrow points at %s")                                                                      \
    X(TutorialPointsAtIn, "the arrow points at %s, in %s")                                                             \
    X(TutorialHide, "hide the tutorial text")                                                                          \
    X(TipAvailable, "help system tip available: Control+H reads it")                                                   \
    X(GroupTip, "help tip")                                                                                            \
    X(NoTip, "no help tip right now")                                                                                  \
    X(TutorialPointsAtSpot, "the arrow points at a spot on the lot: %s")                                               \
    X(SpotOnLot, "spot")                                                                                               \
    X(HintNewspaper,                                                                                                   \
      "Keyboard: Tab to things by kind, Enter on miscellaneous or other objects, Down to the newspaper, Enter, "       \
      "choose Find a job.")                                                                                            \
    X(TutorialEscapeAgain, "Escape again leaves the tutorial")                                                         \
    X(TutorialOver, "the tutorial is over")                                                                            \
    /* The tutorial's steps told for the keyboard (nav/tutorial.cpp) */                                                \
    X(StepZoom,                                                                                                        \
      "Take a look at our tutorial house. Press the Zoom in button: Tab to controls, arrows to Zoom in, Enter. You "   \
      "can press Escape twice at any time to cancel the tutorial.")                                                    \
    X(StepZoomAgain, "This is the medium zoom level. Now press Enter on Zoom in again to get even closer.")            \
    X(StepScroll,                                                                                                      \
      "There are many ways to scroll the screen. With the keyboard the camera follows what you browse: Tab to "        \
      "things by kind, Enter on a kind, then Down goes to the next object.")                                           \
    X(StepScrollAgain,                                                                                                 \
      "Good. The arrows go through the objects, Backspace goes back to the kinds, and the camera goes to each one. "   \
      "Press Down again.")                                                                                             \
    X(StepRotate, "Now let us rotate the point of view: in controls, press Rotate right.")                             \
    X(StepLive,                                                                                                        \
      "You know how to control the view; time for Live mode. Press F1, or Live mode in controls. If it is selected "   \
      "already, press Buy mode (F2) first and then Live mode.")                                                        \
    X(StepPortrait,                                                                                                    \
      "The family members are on the control panel below. This family is one person, Bob. Press Control+1, his "       \
      "portrait, to center him on the screen.")                                                                        \
    X(StepGoHere,                                                                                                      \
      "Now you will learn to control Bob. Choose an object in things by kind or by room and press Control+Enter: "     \
      "Bob walks to that spot.")                                                                                       \
    X(StepGoHereAgain, "Press Control+Enter on an object so that Bob goes there.")                                     \
    X(StepInteract,                                                                                                    \
      "You can order Sims to interact with most objects. In things by kind, the arrows go through the objects; "       \
      "Enter on one opens its interactions, and Enter chooses one for Bob.")                                           \
    X(StepMood,                                                                                                        \
      "The mood tab, in the Sim panels, is both a button and a meter: green means Bob is happy, red that he is "       \
      "not. Tab to the Sim panels, Enter on Mood to see his needs.")                                                   \
    X(StepShower,                                                                                                      \
      "Let us improve Bob's hygiene. Tab to things by kind, Enter on plumbing, Down to the shower, Enter, then "       \
      "choose Take a shower.")                                                                                         \
    X(StepNeeds,                                                                                                       \
      "Go through the needs: Tab to Needs, up and down, Enter on one reads its explanation and which objects "         \
      "satisfy it. Objects satisfy needs with different efficiency.")                                                  \
    X(StepBuy,                                                                                                         \
      "If you keep most needs green, your Sim will be satisfied; usually you satisfy the most critical first. Now "    \
      "Buy mode: press F2, or Buy mode in controls.")                                                                  \
    X(StepChooseChair,                                                                                                 \
      "Take a look at the catalog: Tab to catalog, Enter on Seating, arrows along the chairs; Enter on the one Bob "   \
      "would like starts placing it.")                                                                                 \
    X(StepPlace,                                                                                                       \
      "To buy it: Tab and Shift+Tab choose a free spot, Enter buys it there. Escape puts away the next one that "      \
      "stays in hand.")                                                                                                \
    X(StepTurn,                                                                                                        \
      "Great, now let us rotate it: in things by kind, Enter on seating, go to the chair just bought, Enter picks "    \
      "it up, the period key turns it, Enter puts it down.")                                                           \
    X(StepBackToLive,                                                                                                  \
      "Remember you can always return to Buy mode to buy new objects or rearrange the house. Now back to Live "        \
      "mode: F1, or Live mode in controls.")                                                                           \
    X(StepPersonality,                                                                                                 \
      "If you give no orders, Sims decide on their own, by their needs and personality. Let us look at Bob's "         \
      "personality: Tab to the Sim panels, arrows to Personality, Enter.")                                             \
    X(StepPersonalityTab, "Press Enter on the Personality tab to see the personality panel.")                          \
    X(StepSecondSim,                                                                                                   \
      "Now let us add one more person to this family: Betty. Press Control+2 to select her, and Control+2 again to "   \
      "center her on the screen.")                                                                                     \
    X(StepSecondObject,                                                                                                \
      "Choose an object in things by kind or by room and press Enter so that Betty does something; the "               \
      "interactions are hers now.")                                                                                    \
    X(StepSelectFirst, "There are several ways to change the active Sim. Press Control+1 to select Bob.")              \
    X(StepSpace,                                                                                                       \
      "Very good, Bob is selected. Another way to choose a Sim is the space bar. Press Space now and you are back "    \
      "to Betty.")                                                                                                     \
    X(StepSocial,                                                                                                      \
      "Good. Sims interact with each other like with objects. With Betty selected: Tab to things by kind, Enter on "   \
      "Sims, Down to Bob, Enter, and choose an interaction.")                                                          \
    X(StepSocialAgain,                                                                                                 \
      "How the Sims feel about each other is in the Relationships panel, with daily and lifetime scores. Try "         \
      "another interaction with Bob and see the result there.")                                                        \
    X(StepRelationships,                                                                                               \
      "Tab to the Sim panels, arrows to Relationships, Enter: it shows how the selected Sim feels about this "         \
      "person.")                                                                                                       \
    X(StepNewspaper,                                                                                                   \
      "Sims earn money by working. Find the newspaper outside: Tab to things by kind, Enter on other objects, Down "   \
      "to Newspaper, Enter, and choose Find a job.")                                                                   \
    X(StepJobTab, "Congratulations! Now Tab to the Sim panels, arrows to Job, Enter to find out about the new job.")   \
    X(StepJobName,                                                                                                     \
      "This panel shows the selected Sim's job and skill levels, important for promotions. Tab to job, Enter on "      \
      "the job name to learn when the car pool comes.")                                                                \
    X(StepSkills,                                                                                                      \
      "The skills (Cooking, Mechanical...) read with their points; one that needs improving for a promotion says "     \
      "so. Arrows to a skill, Enter to learn how to gain it.")                                                         \
    X(StepOptions,                                                                                                     \
      "Options mode is where you save, quit or return to the neighborhood, and choose graphics, sound and game "       \
      "options. Press F5, or Options mode in controls.")                                                               \
    /* Buy mode: catalog */                                                                                            \
    X(CannotAfford, "can't afford")                                                                                    \
    X(NoFamilyFunds, "no family, unlimited funds")                                                                     \
    X(RatingHunger, "hunger")                                                                                          \
    X(RatingComfort, "comfort")                                                                                        \
    X(RatingHygiene, "hygiene")                                                                                        \
    X(RatingBladder, "bladder")                                                                                        \
    X(RatingEnergy, "energy")                                                                                          \
    X(RatingFun, "fun")                                                                                                \
    X(RatingRoom, "room")                                                                                              \
    X(SkillCooking, "+cooking")                                                                                        \
    X(SkillMechanical, "+mechanical")                                                                                  \
    X(SkillLogic, "+logic")                                                                                            \
    X(SkillBody, "+body")                                                                                              \
    X(SkillCreativity, "+creativity")                                                                                  \
    X(SkillCharisma, "+charisma")                                                                                      \
    X(SkillStudy, "+study")                                                                                            \
    /* Buy mode: placing */                                                                                            \
    X(AreaSpots, "%s, %d spots")                                                                                       \
    X(AreaSpotsOne, "%s, 1 spot")                                                                                      \
    X(AreaNoSpot, "%s, no spot: %s")                                                                                   \
    X(SpotOf, "%d of %d: %s")                                                                                          \
    X(NoSpotAnywhere, "no spot on this floor: %s")                                                                     \
    X(NoOtherTurn, "no other direction works here")                                                                    \
    X(Bought, "bought %s")                                                                                             \
    X(FundsLeft, "%s left")                                                                                            \
    X(AnotherInHand, "another one in hand, Escape to stop")                                                            \
    X(PlaceFailed, "could not place it: %s")                                                                           \
    X(PlacingCancelled, "purchase cancelled")                                                                          \
    X(Moving, "moving %s, sells for %s")                                                                               \
    X(PutDown, "placed: %s")                                                                                           \
    X(PutBack, "%s put back")                                                                                          \
    X(Sold, "sold %s for %s, you have %s")                                                                             \
    X(ThrownAway, "%s thrown away")                                                                                    \
    X(CannotSell, "it cannot be sold")                                                                                 \
    X(CannotPickUp, "cannot pick it up. %s")                                                                           \
    X(AreaChecked,                                                                                                     \
      "%s: %d good spots; %d do not fit, %d cannot be reached to use it, %d would block a door, %d would leave "       \
      "another object unusable")                                                                                       \
    X(MostlyBecause, "mostly: %s")                                                                                     \
    X(UsedFromTiles, "used from %d tiles")                                                                             \
    X(UsedFromTile, "used from 1 tile")                                                                                \
    X(RoomEmpty, "empty room, %d tiles")                                                                               \
    X(InCorner, "in the %s corner")                                                                                    \
    X(AgainstWall, "back to the %s wall")                                                                              \
    X(BesideWall, "beside the %s wall")                                                                                \
    X(AwayFromWalls, "away from the walls")                                                                            \
    X(FacingDirection, "facing %s")                                                                                    \
    X(FacingObject, "facing %s")                                                                                       \
    X(NextToObject, "next to %s")                                                                                      \
    X(OnTopOf, "on %s")                                                                                                \
    X(MeantFor, "meant for: %s")                                                                                       \
    X(CutsOff, "cuts off %d tiles")                                                                                    \
    X(WouldBlockObject, "it would leave %s unusable")                                                                  \
    X(NoReachableSpot, "no Sim could get to it")                                                                       \
    X(WouldBlockDoor, "it would block a door")                                                                         \
    X(DoesNotFit, "it does not fit")                                                                                   \
    X(PlaceOnSurface, "it goes on a table, a counter or another surface")                                              \
    X(ObjectSize, "takes %1$d tiles, %2$d by %3$d")                                                                    \
    X(NoGroupLetter, "no group starting with %s")                                                                      \
    X(KeysGroupLetter, "Alt and a letter: the next group whose name starts with that letter.")                         \
    /* Default lot objects that have no catalog name */                                                                \
    X(ObjMailbox, "mailbox")                                                                                           \
    X(ObjTrashOutside, "outdoor trash can")                                                                            \
    X(ObjTrashPile, "pile of trash")                                                                                   \
    X(ObjPineTree, "pine tree")                                                                                        \
    X(ObjBirchTree, "birch tree")                                                                                      \
    X(ObjSycamoreTree, "sycamore")                                                                                     \
    X(ObjOakTree, "oak")                                                                                               \
    X(ObjMapleTree, "maple")                                                                                           \
    X(ObjPalmTree, "palm tree")                                                                                        \
    X(ObjWillowTree, "willow")                                                                                         \
    X(ObjAppleTree, "apple tree")                                                                                      \
    X(ObjCherryTree, "cherry tree")                                                                                    \
    X(ObjElmTree, "elm")                                                                                               \
    X(ObjCypressTree, "cypress")                                                                                       \
    X(ObjTree, "tree")                                                                                                 \
    /* Lot: status (the HUD) */                                                                                        \
    X(GroupStatus, "status")                                                                                           \
    X(GroupFamily, "family")                                                                                           \
    X(LabelValue, "%s: %s")                                                                                            \
    /* Lot: the selected Sim */                                                                                        \
    X(MoodGood, "%s, %d of 5 bars green")                                                                              \
    X(MoodBad, "%s, %d of 5 bars red")                                                                                 \
    X(MoodNeutral, "%s, neutral")                                                                                      \
    X(RelationshipValues, "%s. Daily %d, lifetime %d")                                                                 \
    X(SkillValue, "%s, %d of 10")                                                                                      \
    X(GroupJob, "job")                                                                                                 \
    X(JobPay, "pay")                                                                                                   \
    X(JobFriendsNeeded, "family friends")                                                                              \
    X(JobPerformance, "performance")                                                                                   \
    X(GroupHouse, "house")                                                                                             \
    /* Text fields */                                                                                                  \
    X(TextField, "%s %s, text field")                                                                                  \
    X(Blank, "blank")                                                                                                  \
    X(Space, "space")                                                                                                  \
    /* Create a Sim */                                                                                                 \
    X(OutOfTen, "%d of 10")                                                                                            \
    X(ValueOfMax, "%d of %d")                                                                                          \
    X(PointsLeft, "%d points left")                                                                                    \
    X(PointLeft, "1 point left")                                                                                       \
    X(NoPointsLeft, "no points left")                                                                                  \
    X(HeadLook, "head %d of %d: %s")                                                                                   \
    X(BodyLook, "body %d of %d, %s: %s")                                                                               \
    X(BodyFit, "fit")                                                                                                  \
    X(BodyFat, "heavy")                                                                                                \
    X(BodySkinny, "thin")                                                                                              \
    X(BodyLookNoShape, "body %d of %d: %s")                                                                            \
    X(SimPreview, "Sim preview")                                                                                       \
    X(GroupPersonality, "personality")                                                                                 \
    X(GroupLooks, "appearance")                                                                                        \
    /* Family picker */                                                                                                \
    X(GroupMembers, "members")                                                                                         \
    X(TraitNeat, "Neat")                                                                                               \
    X(TraitOutgoing, "Outgoing")                                                                                       \
    X(TraitActive, "Active")                                                                                           \
    X(TraitPlayful, "Playful")                                                                                         \
    X(TraitNice, "Nice")                                                                                               \
    /* Balloons over Sims and objects, visitors, taken objects (nav/narrator.cpp) */                                   \
    X(BalloonThinks, "%1$s thinks: %2$s")                                                                              \
    X(BalloonShouts, "%1$s shouts: %2$s")                                                                              \
    X(BalloonSays, "%1$s says: %2$s")                                                                                  \
    X(BalloonShows, "%1$s: %2$s")                                                                                      \
    X(BalloonRouteBlocked, "%1$s can't get past %2$s")                                                                 \
    X(BalloonRouteFailed, "%1$s can't get there: %2$s")                                                                \
    X(ShownThinks, "thinks: %s")                                                                                       \
    X(ShownShouts, "shouts: %s")                                                                                       \
    X(ShownSays, "says: %s")                                                                                           \
    X(ShownRouteBlocked, "can't get past %s")                                                                          \
    X(ShownRouteFailed, "can't get there: %s")                                                                         \
    X(BalloonCrossedOut, "%s, crossed out")                                                                            \
    X(NeedSocial, "social")                                                                                            \
    X(InterestTravel, "Travel")                                                                                        \
    X(InterestMoney, "Money")                                                                                          \
    X(InterestPolitics, "Politics")                                                                                    \
    X(InterestSixties, "The 60's")                                                                                     \
    X(InterestWeather, "Weather")                                                                                      \
    X(InterestSports, "Sports")                                                                                        \
    X(InterestMusic, "Music")                                                                                          \
    X(InterestOutdoors, "Outdoors")                                                                                    \
    X(InterestToys, "Toys")                                                                                            \
    X(InterestAliens, "Aliens")                                                                                        \
    X(InterestPets, "Pets")                                                                                            \
    X(InterestSchool, "School")                                                                                        \
    X(InterestExercise, "Exercise")                                                                                    \
    X(InterestFood, "Food")                                                                                            \
    X(InterestParties, "Parties")                                                                                      \
    X(InterestStyle, "Style")                                                                                          \
    X(InterestHollywood, "Hollywood")                                                                                  \
    X(InterestTechnology, "Technology")                                                                                \
    X(InterestRomance, "Romance")                                                                                      \
    X(TopicWork, "work, %s")                                                                                           \
    X(CareerBusiness, "business")                                                                                      \
    X(CareerEntertainment, "entertainment")                                                                            \
    X(CareerLaw, "law enforcement")                                                                                    \
    X(CareerCrime, "crime")                                                                                            \
    X(CareerMedicine, "medicine")                                                                                      \
    X(CareerMilitary, "military")                                                                                      \
    X(CareerPolitics, "politics")                                                                                      \
    X(CareerAthletics, "sports")                                                                                       \
    X(CareerScience, "science")                                                                                        \
    X(CareerXtreme, "extreme sports")                                                                                  \
    X(CareerMusic, "music")                                                                                            \
    X(CareerSlacker, "slacker")                                                                                        \
    X(CareerParanormal, "paranormal")                                                                                  \
    X(CareerJournalism, "journalism")                                                                                  \
    X(CareerHacker, "hacker")                                                                                          \
    X(CareerUnemployed, "unemployed")                                                                                  \
    X(TopicBored, "bored")                                                                                             \
    X(TopicBadMood, "bad mood")                                                                                        \
    X(TopicMagic, "magic")                                                                                             \
    X(BalloonRomance, "romance")                                                                                       \
    X(BalloonFriends, "friends")                                                                                       \
    X(HeadlineHate, "hate")                                                                                            \
    X(HeadlineStress, "stress")                                                                                        \
    X(HeadlineSurprise, "surprise")                                                                                    \
    X(HeadlineIdea, "idea")                                                                                            \
    X(HeadlineLove, "love")                                                                                            \
    X(HeadlineDrunk, "drunk")                                                                                          \
    X(HeadlineHurt, "hurt")                                                                                            \
    X(HeadlineSmell, "bad smell")                                                                                      \
    X(HeadlineFlies, "flies")                                                                                          \
    X(SocialHug, "Hug")                                                                                                \
    X(SocialGift, "Give Gift")                                                                                         \
    X(SocialBackRub, "Give Back Rub")                                                                                  \
    X(SocialCheerUp, "Cheer Up")                                                                                       \
    X(SocialEntertain, "Entertain")                                                                                    \
    X(SocialTickle, "Tickle")                                                                                          \
    X(SocialBrag, "Brag")                                                                                              \
    X(SocialSlap, "Slap")                                                                                              \
    X(SocialScare, "Scare")                                                                                            \
    X(SocialTease, "Tease")                                                                                            \
    X(SocialAttack, "Attack")                                                                                          \
    X(BalloonNeedsChair, "needs a chair")                                                                              \
    X(BalloonInUse, "in use")                                                                                          \
    X(BalloonCantGetThere, "can't get there")                                                                          \
    X(BalloonRelationUpMuch, "relationship up a lot")                                                                  \
    X(BalloonRelationUp, "relationship up")                                                                            \
    X(BalloonRelationDown, "relationship down")                                                                        \
    X(BalloonRelationDownMuch, "relationship down a lot")                                                              \
    X(BalloonPizza, "pizza")                                                                                           \
    X(BalloonCrush, "crush")                                                                                           \
    X(BalloonFameUpMuch, "fame up a lot")                                                                              \
    X(BalloonFameUp, "fame up")                                                                                        \
    X(BalloonFameDown, "fame down")                                                                                    \
    X(BalloonFameDownMuch, "fame down a lot")                                                                          \
    X(RouteNoChair, "no chair found")                                                                                  \
    X(RouteNoDoor, "no door found")                                                                                    \
    X(RouteWall, "a wall is in the way")                                                                               \
    X(RouteNotLevel, "the ground is not level")                                                                        \
    X(RouteNoLadder, "no pool ladder")                                                                                 \
    X(ProgressSkill, "skill %d percent")                                                                               \
    X(ProgressRepair, "repair %d percent")                                                                             \
    X(ProgressInterest, "interest %d percent")                                                                         \
    X(VisitorArrived, "%s arrived")                                                                                    \
    X(VisitorLeft, "%s left")                                                                                          \
    X(GhostAppeared, "the ghost of %s appeared")                                                                       \
    X(GhostLeft, "the ghost of %s went away")                                                                          \
    X(ObjectTakenBy, "%1$s took %2$s, worth %3$s")                                                                     \
    X(ObjectTakenByNoValue, "%1$s took %2$s")                                                                          \
    X(ObjectGone, "%1$s is gone, worth %2$s")                                                                          \
    X(ObjectGoneNoValue, "%s is gone")                                                                               \
    /* The family, by ear */                                                                                         \
    X(SimNowIn, "%1$s, now in %2$s")                                                                                 \
    X(ActionStarted, "%1$s: %2$s")                                                                                   \
    X(ActionStartedOwn, "%1$s, on their own: %2$s")                                                                  \
    X(RelationChanged, "%1$s with %2$s: %3$s, now %4$d")                                                             \
    X(ActionFinished, "%1$s finished %2$s")                                                                          \
    X(NeedLow, "%1$s, low need: %2$s")                                                                               \
    X(NeedsLow, "%1$s, low needs: %2$s")                                                                             \
    X(NeedUrgent, "%1$s, urgent need: %2$s")                                                                         \
    X(NeedsUrgent, "%1$s, urgent needs: %2$s")                                                                       \
    X(NeedRecovered, "%1$s, need met again: %2$s")                                                                   \
    X(MoodTurnedBad, "%s is in a bad mood")                                                                          \
    X(MoodGoodAgain, "%s is in a good mood again")                                                                   \
    X(NarratorEverything, "narrator: everything")                                                                    \
    X(NarratorWarnings, "narrator: warnings only")                                                                   \
    X(NarratorOff, "narrator off; events are still kept")                                                            \
    X(NoSimSelected, "no Sim selected")                                                                              \
    X(StatusInRoom, "in %s")                                                                                         \
    X(StatusIdle, "doing nothing")                                                                                   \
    X(StatusThen, "then %s")                                                                                         \
    X(StatusNeedLow, "low need: %s")                                                                                 \
    X(StatusNeedsLow, "low needs: %s")                                                                               \
    X(StatusNeedUrgent, "urgent need: %s")                                                                           \
    X(StatusNeedsUrgent, "urgent needs: %s")                                                                         \
    X(StatusNeedsFine, "needs fine")                                                                                 \
    X(StatusMood, "mood")                                                                                            \
    /* Sounds */                                                                                                     \
    X(SoundCarpool, "the carpool is honking")                                                                        \
    X(SoundSchoolBus, "the school bus is honking")                                                                   \
    X(SoundPoliceCar, "a police car is honking")                                                                     \
    /* Build cursor (walls) */                                                                                       \
    X(BuildNorth, "north")                                                                                           \
    X(BuildEast, "east")                                                                                             \
    X(BuildSouth, "south")                                                                                           \
    X(BuildWest, "west")                                                                                             \
    X(BuildRoomSize, "%1$s, %2$d tiles")                                                                             \
    X(BuildSteps, "%1$d %2$s")                                                                                       \
    X(BuildFromStart, "from the start: %s")                                                                          \
    X(BuildAtStart, "at the start")                                                                                  \
    X(BuildWallPlan, "Enter builds a wall of %1$d, %2$s")                                                            \
    X(BuildDiagonalPlan, "Enter builds a diagonal wall of %1$d, %2$s")                                               \
    X(BuildRoomPlan, "Shift+Enter builds a room %1$d by %2$d, %3$s")                                                 \
    X(BuildCorner, "%1$d, %2$d")                                                                                     \
    X(BuildWallsHere, "walls to the %s")                                                                             \
    X(BuildFreeSpace, "free space: %1$d north, %2$d east, %3$d south, %4$d west")                                    \
    X(BuildStartAt, "start at %1$d, %2$d")                                                                           \
    X(BuildStyle, "%1$s, %2$s per section")                                                                          \
    X(BuildBlocksObjects, "Sims could no longer reach: %s")                                                          \
    X(BuildBlocksDoor, "it closes a door")                                                                           \
    X(BuildRoomNeedsDoor, "the new room has %d tiles and no door yet: add one with the door tool")                   \
    X(BuildCutsOff, "it cuts off %d tiles from the rest of the lot")                                                 \
    X(BuildWarning, "Warning: %s. Enter again to build anyway.")                                                     \
    X(BuildFailed, "could not build")                                                                                \
    X(BuildNeedsArea, "a room needs width and depth: move away from the start both across and down")                 \
    X(BuiltRoom, "room built, %1$d by %2$d")                                                                         \
    X(BuildCursorStart, "Build cursor, %s. Control+F1 says the keys.")                                               \
    X(BuildLotEdge, "edge of the lot")                                                                               \
    X(BuildStartCleared, "start cleared")                                                                            \
    X(BuildStartMarked, "start marked at %1$d, %2$d")                                                                \
    X(BuildNoStart, "mark a start first with Space")                                                                 \
    X(BuildRemoved, "walls removed, %d sections")                                                                    \
    X(BuildNotStraight, "a wall goes straight or diagonal; Shift+Enter builds a room instead")                       \
    X(BuiltWall, "wall built, %d sections")                                                                          \
    X(BuiltDiagonal, "diagonal wall built, %d sections")                                                             \
    X(BuildPreset, "room %1$d by %2$d, %3$d tiles, %4$s; Insert builds it with its north-west corner here")          \
    X(BuildPresetTooBig, "that room does not fit here: move the cursor north or west")                               \
    X(BuildCursorEnd, "build cursor off")                                                                            \
    X(KeysBuildCursor,                                                                                               \
      "Arrows move the cursor one corner, with Shift five. The cursor says where it is, the walls that meet "        \
      "there and what stands there; R says everything, including the free space each way and the room. "             \
      "Space marks the start; from then on the cursor says how far it is from the start and what Enter "             \
      "would build and cost. Enter builds a wall from the start to the cursor, Shift+Enter builds the four "         \
      "walls of a room between them, Control+Enter or Delete removes walls. Before building, the mod warns "         \
      "when the walls would cut Sims off from an object, close a door or shut off part of the lot, and "             \
      "Enter again builds anyway. Page Down and Page Up choose a ready-made room size, Insert builds it "            \
      "with its north-west corner at the cursor. Home goes back to the selected Sim, Escape leaves the "             \
      "cursor.")                                                                                                     \
    /* Build cursor (area) */                                                                                        \
    X(BuildOutsideArea, "nothing built: that is outside the lot's buildable area")                                   \
    X(BuildNotBuildableHere, "outside the buildable area")                                                           \
    /* Build cursor (cost) */                                                                                        \
    X(BuildCostWas, "cost %s")                                                                                       \
    /* Build cursor (trapped) */                                                                                     \
    X(BuildTrapsSims, "it would shut in: %s")                                                                        \
    /* Doors and windows */                                                                                          \
    X(DoorInWall, "in the %1$s wall of %2$s, opening onto %3$s")                                                     \
    X(DoorFirst, "the first door of that room")                                                                      \
    /* Floor cursor */                                                                                               \
    X(FloorHere, "floor")                                                                                            \
    X(FloorNone, "no floor")                                                                                         \
    X(FloorPlan, "Enter floors %1$d by %2$d, %3$d tiles, %4$s")                                                      \
    X(FloorCursorStart, "Floor cursor, %s. Control+F1 says the keys.")                                               \
    X(FloorNothing, "nothing changed there")                                                                         \
    X(FloorRoomCleared, "floor taken off the whole room")                                                            \
    X(FloorRoomLaid, "floor laid in the whole room")                                                                 \
    X(FloorRemoved, "floor removed, %d tiles")                                                                       \
    X(FloorLaid, "floor laid, %d tiles")                                                                             \
    X(FloorNoPresets, "ready-made rooms are for the wall tool")                                                      \
    X(KeysFloorCursor,                                                                                               \
      "Arrows move the cursor one tile, with Shift five; each tile says whether it has floor and what "              \
      "stands there, R says everything. Enter floors the tile under the cursor; with a start marked by "             \
      "Space, the whole rectangle from the start to the cursor, whose size and price the cursor says as it "         \
      "moves. Shift+Enter floors the whole room under the cursor. Control+Enter removes floor, "                     \
      "Control+Shift+Enter from the whole room. Home goes back to the selected Sim, Escape leaves the "              \
      "cursor.")                                                                                                     \
    /* Floor cursor (price) */                                                                                       \
    X(FloorStyle, "%1$s, %2$s per tile")                                                                             \
    X(FloorTiles, "%d tiles")                                                                                        \
    /* Wallpaper cursor */                                                                                           \
    X(PaperPlan, "Enter papers every wall of %s")                                                                    \
    X(PaperCursorStart, "Wallpaper cursor, %s. Control+F1 says the keys.")                                           \
    X(PaperNoWalls, "this room has no walls to paper")                                                               \
    X(PaperDone, "room papered, %d wall sides")                                                                      \
    X(PaperStripped, "wallpaper taken off, %d wall sides")                                                           \
    X(PaperNoStart, "wallpaper goes on whole rooms: no start is needed")                                             \
    X(KeysPaperCursor,                                                                                               \
      "Arrows move the cursor one tile, with Shift five; each tile says its room. Enter papers every wall "          \
      "of the room under the cursor, inside that room; on the outdoors, the outside walls. Control+Enter "           \
      "takes the wallpaper off the room's walls. R says everything, Home goes back to the selected Sim, "            \
      "Escape leaves the cursor.")                                                                                   \
    /* Pool cursor */                                                                                                \
    X(PoolPlan, "Enter digs a pool %1$d by %2$d, %3$d tiles, %4$s")                                                  \
    X(PoolCursorStart, "Pool cursor, %s. Control+F1 says the keys.")                                                 \
    X(PoolBuilt, "pool dug, %d tiles")                                                                               \
    X(PoolRemoved, "pool filled in, %d tiles")                                                                       \
    X(PoolNeedsLadder, "Sims need a pool ladder to get in and out: it is in this tool's list")                       \
    X(KeysPoolCursor,                                                                                                \
      "Arrows move the cursor one tile, with Shift five. Space marks one corner of the pool; the cursor "            \
      "then says its size and price. Enter digs the pool from the start to the cursor, or on the tile "              \
      "alone; before digging, the mod warns if the water would cut Sims off from an object, block a door, "          \
      "shut someone in or isolate part of the lot, and Enter again digs anyway. Control+Enter fills pool "           \
      "tiles back in. Ladders, diving boards and slides are placed from the list like objects. Home goes "           \
      "back to the selected Sim, Escape leaves the cursor.")                                                         \
    /* Terrain cursor */                                                                                             \
    X(LevelPlan, "Enter levels %1$d by %2$d to the height at the start")                                             \
    X(TerrainCursorStart, "Terrain cursor, %s. Control+F1 says the keys.")                                           \
    X(TerrainChanged, "done")                                                                                        \
    X(TerrainLeveled, "ground levelled, %1$d by %2$d")                                                               \
    X(KeysTerrainCursor,                                                                                             \
      "Arrows move the cursor one tile, with Shift five. With the level tool, Space marks the tile whose "           \
      "height is kept, and Enter levels everything from there to the cursor: walls, floors and most objects "        \
      "need level ground, and the game says when it is not. With raise or lower, Enter raises or lowers the "        \
      "ground once at the cursor. Home goes back to the selected Sim, Escape leaves the cursor.")                    \
    /* Terrain (heights) */                                                                                          \
    X(TerrainHeight, "height %d")                                                                                    \
    X(TerrainSloped, "sloped")                                                                                       \
    /* Room layout (Ctrl+L) */                                                                                       \
    X(LayoutUnknown, "no room here")                                                                                 \
    X(LayoutOutside, "%1$s, %2$d tiles")                                                                             \
    X(LayoutSize, "%1$s, %2$d by %3$d, %4$d tiles, from %5$d, %6$d to %7$d, %8$d")                                   \
    X(LayoutOpening, "%1$s wall, to %2$s")                                                                           \
    X(LayoutDoors, "doors: %s")                                                                                      \
    X(LayoutNoDoors, "no doors")                                                                                     \
    X(LayoutWindows, "windows: %s")                                                                                  \
    X(LayoutNoWindows, "no windows")                                                                                 \
    X(LayoutAlongWall, "along the %1$s wall: %2$s")                                                                  \
    X(LayoutMiddle, "in the middle: %s")                                                                             \
    X(LayoutNothing, "nothing in it yet")                                                                            \
    X(LayoutMore, "Control+Up says what is in it")                                                                   \
    X(KeysLayoutHint, "Control+L describes the room: its size, doors, windows and what stands along each wall.")     \
    /* House overview (build mode, Ctrl+Shift+L) */                                                                  \
    X(HouseRoomItem, "%1$s, %2$d tiles")                                                                             \
    X(HouseNone, "No house yet: the lot is all outdoors, %d tiles")                                                  \
    X(HouseSummary, "The house: %1$d floors, %2$d rooms, %3$d tiles indoors; %4$d tiles outdoors")                   \
    X(HouseFloorRooms, "floor %1$d: %2$s")                                                                           \
    X(HouseRateSize, "Size")                                                                                         \
    X(HouseRateFurnishings, "Furnishings")                                                                           \
    X(HouseRateYard, "Yard")                                                                                         \
    X(HouseRateUpkeep, "Upkeep")                                                                                     \
    X(HouseRateLayout, "Layout")                                                                                     \
    X(HouseRating, "%1$s %2$d")                                                                                      \
    X(HouseRatings, "house ratings, out of 10: %s")                                                                  \
    X(NeedToilet, "a toilet")                                                                                        \
    X(NeedWash, "a shower or bath")                                                                                  \
    X(NeedFridge, "a fridge")                                                                                        \
    X(NeedStove, "a stove")                                                                                          \
    X(NeedPhone, "a phone")                                                                                          \
    X(NeedSmokeAlarm, "a smoke alarm")                                                                               \
    X(HouseMissing, "missing: %s")                                                                                   \
    X(HouseBeds, "beds for %1$d of %2$d Sims")                                                                       \
    X(HouseClosedRooms, "no way in: %s")                                                                             \
    X(HouseUnreachable, "Sims cannot reach: %s")                                                                     \
    X(HouseDark, "dark, with no window or lamp: %s")                                                                 \
    X(HouseDisliked, "the Sims dislike being in: %s")                                                                \
    X(HouseLiked, "the Sims like being in: %s")                                                                      \
    X(HouseAllGood, "nothing missing, nothing blocked")                                                              \
    X(HouseToCheck, "%d things to check: Control+Up reads them")                                                     \
    /* House overview (one floor) */                                                                                 \
    X(HouseSummaryOneFloor, "The house: one floor, %1$d rooms, %2$d tiles indoors; %3$d tiles outdoors")             \
    X(HouseSummaryOneRoom, "The house: one room, %1$d tiles; %2$d tiles outdoors")                                   \
    /* Wardrobe */                                                                                                   \
    X(OutfitLook, "outfit %d of %d, %s: %s")                                                                         \
    X(OutfitLookNoShape, "outfit %d of %d: %s")                                                                      \
    /* Wardrobe */                                                                                                   \
    X(OutfitNotListed, "the outfit worn now, %d to choose from")                                                     \
    /* Travel maps */                                                                                                \
    X(ScreenOldTown, "Old Town, where to?")                                                                          \
    X(LotFilters, "in the filters: %s")                                                                              \
    X(LotsMarked, "marked on the map: %s")                                                                           \
    X(LotsMarkedNone, "no place of this kind on the map")                                                            \
    /* Pet adoption */                                                                                               \
    X(PetLook, "pet %d of %d: %s")                                                                                   \
    X(BreedLabrador, "Labrador retriever")                                                                           \
    X(BreedAussie, "Australian shepherd")                                                                            \
    X(BreedDalmatian, "Dalmatian")                                                                                   \
    X(BreedCairn, "Cairn terrier")                                                                                   \
    X(BreedPoodle, "poodle")                                                                                         \
    X(BreedJackRussell, "Jack Russell terrier")                                                                      \
    X(BreedSpringer, "springer spaniel")                                                                             \
    X(BreedSetter, "setter")                                                                                         \
    X(BreedGreyhound, "greyhound")                                                                                   \
    X(BreedBeagle, "beagle")                                                                                         \
    X(BreedRottweiler, "Rottweiler")                                                                                 \
    X(BreedBulldog, "bulldog")                                                                                       \
    X(BreedScottie, "Scottish terrier")                                                                              \
    X(BreedShepherd, "German shepherd")                                                                              \
    X(BreedChihuahua, "Chihuahua")                                                                                   \
    X(BreedPinkPoodle, "pink poodle")                                                                                \
    X(BreedTabby, "tabby")                                                                                           \
    X(BreedSilverTabby, "silver tabby")                                                                              \
    X(BreedCalico, "calico")                                                                                         \
    X(BreedPersian, "Persian")                                                                                       \
    X(BreedBlueManx, "blue Manx")                                                                                    \
    X(BreedRagdoll, "Ragdoll")                                                                                       \
    X(BreedSiamese, "Siamese")                                                                                       \
    X(BreedTortoiseshell, "tortoiseshell")                                                                           \
    X(BreedBombay, "Bombay")                                                                                         \
    X(BreedSpotted, "spotted")                                                                                       \
    X(BreedBlackCalico, "black calico")                                                                              \
    X(BreedRedCalico, "red calico")                                                                                  \
    X(BreedSphynx, "Sphynx")                                                                                         \
    X(BreedWhite, "white")                                                                                           \
    X(BreedBrownTabby, "brown tabby")                                                                                \
    /* Room kinds (community lots) */                                                                                \
    X(RoomShop, "shop")                                                                                              \
    /* Pet skills */                                                                                                 \
    X(GroupPetSkills, "pet skills")                                                                                  \
    /* Transformation (Makin Magic) */                                                                               \
    X(TransformLook, "look %d of %d")                                                                                \
    /* Build cursor: floors and rooms */                                                                             \
    X(BuildAbove, "above %s")                                                                                        \
    X(BuildAboveNothing, "nothing built below")                                                                      \
    X(BuildNoRooms, "no rooms on this floor or below yet")                                                           \
    X(BuildRoomJump, "%s, %d by %d, at its north-west corner")                                                       \
    X(KeysRoomJump,                                                                                                    \
      "Tab and Shift+Tab, or Control+Page Down and Page Up, jump to the next or previous room, at its north-west "     \
      "corner; on an upper floor, to the rooms below.")                                                                \
    /* Stairs */                                                                                                     \
    X(StairsClimb, "climbs %s")                                                                                      \
    X(StairsTop, "reaches the upper floor at tile %d, %d")                                                           \
    /* Build warnings */                                                                                             \
    X(BuildBlocksStairs, "cuts off the stairs")                                                                      \
    /* Build cursor: floors and rooms */                                                                             \
    X(KeysFloorChange,                                                                                                 \
      "Page Up and Page Down go to the floor above or below; choosing a tool puts you back on the first floor.")       \
    /* Catalog room filter */                                                                                        \
    X(RoomFilterAll, "products for every room")                                                                      \
    X(RoomFilterOn, "only products for: %s")                                                                         \
    X(RoomFilterNoMore, "no more products for %s this way")                                                          \
    X(KeysRoomFilter,                                                                                                \
      "Control+R keeps the catalog to the products meant for one kind of room: kitchen, bathroom, bedroom, "         \
      "living room, dining room, study, outside, or all again.")                                                     \
    /* Family watch: refusals */                                                                                     \
    X(ActionDropped, "%s will not do: %s")                                                                           \
    X(ActionDroppedMood, "%s refuses, in a bad mood: %s")                                                            \
    /* Lot objects */                                                                                                \
    X(ObjDigHole, "hole a dog dug")                                                                                  \
    X(ObjDigPile, "pile of dirt a dog dug up")                                                                       \
    X(ObjRoaches, "cockroaches")                                                                                     \
    /* Narrator: visitors */                                                                                         \
    X(PizzaGreetHint,                                                                                                  \
      "Greet him to get the pizza: in things by kind, Enter on Sims, Down to him, Enter, Greet. Unless someone "       \
      "greets him he leaves with it.")                                                                                 \
    /* Placing: free cursor */                                                                                       \
    X(FreeFits, "fits")                                                                                              \
    X(FreeNoFit, "does not fit: %s")                                                                                 \
    X(KeysPlacingFree,                                                                                                 \
      "The arrows move it one tile at a time instead, Shift with them five, saying whether it fits there; comma "      \
      "and period turn it, Enter puts it there, Page Down goes back to the spots.")                                    \
    /* Sim card */                                                                                                   \
    X(BioFor, "Bio for %s")                                                                                          \
    /* Key help */                                                                                                   \
    X(KeysSimCard, "Control+Shift+1 to 9 open a Sim's card, where its biography can be edited.")                     \
    X(KeysHouseFile,                                                                                                 \
      "On a house, Control+Shift+L describes it as Build Mode does, from its last save: rooms, what is "             \
      "missing, rooms with no way in. Control+Up reads it line by line.")                                            \
    /* Travel maps */                                                                                                \
    X(ScreenStudioTown, "Studio Town, where to?")                                                                    \
    X(ScreenMagicTown, "Magic Town, where to?")                                                                      \
    /* Sounds */                                                                                                     \
    X(SoundTaxi, "the taxi is honking")                                                                              \
    /* Travel maps */                                                                                                \
    X(ScreenVacation, "Vacation Island, where to?")                                                                  \
    /* Sounds */                                                                                                     \
    X(SoundVacationVan, "the vacation van is honking")                                                               \
    /* House overview */                                                                                             \
    X(HouseToCheckOne, "1 thing to check: Control+Up reads it")                                                      \
    /* Credits */                                                                                                    \
    X(CreditsScreen, "Credits. Control+Up reads them line by line, Control+Down goes back; Enter on OK closes.")     \
    /* Travel maps */                                                                                                \
    X(ScreenDowntown, "Downtown, where to?")                                                                         \
    /* Travel maps */                                                                                                \
    X(ViewDowntown, "Downtown view")                                                                                 \
    X(ViewVacation, "Vacation Island view")                                                                          \
    X(ViewStudioTown, "Studio Town view")                                                                            \
    X(ViewMagicTown, "Magic Town view")                                                                              \
    /* Sim panels */                                                                                                 \
    X(GroupReportCard, "Report Card")                                                                                \
    /* Create-a-Sim appearance */                                                                                    \
    X(LookHairBlack, "black hair")                                                                                   \
    X(LookHairBrown, "brown hair")                                                                                   \
    X(LookHairBlond, "blond hair")                                                                                   \
    X(LookHairRed, "red hair")                                                                                       \
    X(LookHairGray, "gray hair")                                                                                     \
    X(LookHairWhite, "white hair")                                                                                   \
    X(LookHairDyed, "dyed hair")                                                                                     \
    X(LookHairVeryShort, "very short")                                                                               \
    X(LookHairShort, "short")                                                                                        \
    X(LookHairMedium, "medium length")                                                                               \
    X(LookHairLong, "long")                                                                                          \
    X(LookSpiky, "spiky")                                                                                            \
    X(LookCurly, "curly")                                                                                            \
    X(LookWavy, "wavy")                                                                                              \
    X(LookSlickedBack, "slicked back")                                                                               \
    X(LookBob, "bob cut")                                                                                            \
    X(LookBun, "in a bun")                                                                                           \
    X(LookAfro, "afro")                                                                                              \
    X(LookMohawk, "mohawk")                                                                                          \
    X(LookPonytail, "ponytail")                                                                                      \
    X(LookBraids, "braids")                                                                                          \
    X(LookPigtails, "pigtails")                                                                                      \
    X(LookBangs, "bangs")                                                                                            \
    X(LookUpdo, "hair up")                                                                                           \
    X(LookBald, "bald")                                                                                              \
    X(LookBeard, "beard")                                                                                            \
    X(LookMoustache, "moustache")                                                                                    \
    X(LookGoatee, "goatee")                                                                                          \
    X(LookStubble, "stubble")                                                                                        \
    X(LookSideburns, "sideburns")                                                                                    \
    X(LookGlasses, "glasses")                                                                                        \
    X(LookSunglasses, "sunglasses")                                                                                  \
    X(LookHat, "hat")                                                                                                \
    X(LookCap, "cap")                                                                                                \
    X(LookHelmet, "helmet")                                                                                          \
    X(LookHeadscarf, "headscarf")                                                                                    \
    X(LookHeadband, "headband")                                                                                      \
    X(LookMask, "mask")                                                                                              \
    X(LookMonster, "monster face")                                                                                   \
    X(LookOlder, "older face")                                                                                       \
    X(LookEarrings, "earrings")                                                                                      \
    X(LookHairFlower, "flower in the hair")                                                                          \
    X(LookArmor, "armor")                                                                                            \
    X(LookUniform, "uniform")                                                                                        \
    X(LookTShirt, "T-shirt")                                                                                         \
    X(LookShirt, "shirt")                                                                                            \
    X(LookTankTop, "tank top")                                                                                       \
    X(LookSweater, "sweater")                                                                                        \
    X(LookHoodie, "hoodie")                                                                                          \
    X(LookJacket, "jacket")                                                                                          \
    X(LookLeatherJacket, "leather jacket")                                                                           \
    X(LookVest, "vest")                                                                                              \
    X(LookCoat, "long coat")                                                                                         \
    X(LookSuit, "suit")                                                                                              \
    X(LookTuxedo, "tuxedo")                                                                                          \
    X(LookTie, "tie")                                                                                                \
    X(LookTrousers, "trousers")                                                                                      \
    X(LookJeans, "jeans")                                                                                            \
    X(LookShorts, "shorts")                                                                                          \
    X(LookOveralls, "overalls")                                                                                      \
    X(LookLederhosen, "lederhosen")                                                                                  \
    X(LookKilt, "kilt")                                                                                              \
    X(LookRobe, "robe")                                                                                              \
    X(LookToga, "toga")                                                                                              \
    X(LookKimono, "kimono")                                                                                          \
    X(LookCostume, "costume")                                                                                        \
    X(LookSportswear, "sports kit")                                                                                  \
    X(LookApron, "apron")                                                                                            \
    X(LookShirtless, "shirtless")                                                                                    \
    X(LookBarefoot, "barefoot")                                                                                      \
    X(LookSandals, "sandals")                                                                                        \
    X(LookBoots, "boots")                                                                                            \
    X(LookSneakers, "sneakers")                                                                                      \
    X(LookHeels, "high heels")                                                                                       \
    X(LookTattoos, "tattoos")                                                                                        \
    X(LookLeather, "leather")                                                                                        \
    X(LookDress, "dress")                                                                                            \
    X(LookLongDress, "long dress")                                                                                   \
    X(LookSkirt, "skirt")                                                                                            \
    X(LookBlouse, "blouse")                                                                                          \
    X(LookSwimsuit, "swimsuit")                                                                                      \
    X(LookBikini, "bikini")                                                                                          \
    X(LookTop, "top")                                                                                                \
    X(LookLeggings, "leggings")                                                                                      \
    X(LookUnderwear, "underwear")                                                                                    \
    X(LookColorBlack, "black")                                                                                       \
    X(LookColorWhite, "white")                                                                                       \
    X(LookColorGray, "gray")                                                                                         \
    X(LookColorRed, "red")                                                                                           \
    X(LookColorBlue, "blue")                                                                                         \
    X(LookColorGreen, "green")                                                                                       \
    X(LookColorYellow, "yellow")                                                                                     \
    X(LookColorOrange, "orange")                                                                                     \
    X(LookColorPurple, "purple")                                                                                     \
    X(LookColorPink, "pink")                                                                                         \
    X(LookColorBrown, "brown")                                                                                       \
    X(LookColorBeige, "beige")                                                                                       \
    X(LookCamouflage, "camouflage")                                                                                  \
    X(LookChecked, "checked")                                                                                        \
    X(LookFloral, "floral")                                                                                          \
    X(LookStriped, "striped")                                                                                        \
    X(LookPatterned, "patterned")                                                                                    \
    /* Makin' Magic book */                                                                                          \
    X(BookIngredients, "ingredients: %s")                                                                            \
    X(GroupSpells, "spells")                                                                                         \
    X(GroupRecipes, "recipes")                                                                                       \
    /* Narrator: conversation topics */                                                                              \
    X(BalloonNotInterested, "%s, not interested")                                                                    \
    /* Narrator: bills */                                                                                            \
    X(BillsArrived, "bills have come: %d unpaid")                                                                    \
    /* Narrator: the Grim Reaper */                                                                                  \
    X(ReaperCame, "%s has come for %s")                                                                              \
    /* Trips: arrival */                                                                                             \
    X(ArrivedAt, "arrived: %s")                                                                                      \
    /* Inventory */                                                                                                  \
    X(ItemCount, "%s, %d of them")                                                                                   \
    /* Trips: home again */                                                                                          \
    X(ArrivedHome, "back home")
