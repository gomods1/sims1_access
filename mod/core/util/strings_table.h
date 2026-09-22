// Every string the mod speaks, in the game's two supported languages here (UTF-8).
// X(id, English, Spanish). Keep ids grouped by feature; printf placeholders must match.
#pragma once

#define SA_STRINGS(X)                                                                              \
    /* Startup */                                                                                  \
    X(ModLoaded, "Sims accessibility loaded", "Accesibilidad de Los Sims cargada")                 \
    /* Key help (Ctrl+F1) */                                                                       \
    X(KeysScreen, "Arrows move within a group, Tab and Shift+Tab change group, Home and End go to the ends, "\
      "Enter presses, R repeats. On a slider or value picker, left and right change it and up and down move. "\
      "Control+Up and Control+Down read the "\
      "details of what was said, Control+Left and Control+Right switch between details and events.",\
      "Las flechas se mueven dentro de un grupo, Tab y Mayúsculas+Tab cambian de grupo, Inicio y Fin van a los "\
      "extremos, Intro pulsa, R repite. En un control deslizante o un selector, izquierda y derecha lo cambian "\
      "y arriba y abajo se mueven. Control+Arriba y "\
      "Control+Abajo leen los detalles de lo dicho, Control+Izquierda y Control+Derecha cambian entre detalles "\
      "y eventos.")                                                                                \
    X(KeysLot, "Up and down change group, left and right move within it; on a settings page up and down go "\
      "through the settings and left and right change a slider or value picker. Page Down and Page Up go through "\
      "objects, Shift with them changes category, Control with them changes room. Enter opens the object's "\
      "interactions, Control+Enter walks there, R repeats. P pauses and reads the time and money. Control+Up "\
      "and Control+Down read the details, Control+Left and Control+Right switch between details and events.",\
      "Arriba y abajo cambian de grupo, izquierda y derecha se mueven dentro; en una página de ajustes arriba y "\
      "abajo recorren los ajustes e izquierda y derecha cambian un control deslizante o un selector. Avanzar y "\
      "Retroceder página recorren los objetos, con Mayúsculas cambian de categoría, con Control cambian de habitación. Intro abre "\
      "las interacciones del objeto, Control+Intro camina hasta allí, R repite. P pausa y lee la hora y el dinero. "\
      "Control+Arriba y Control+Abajo leen los detalles, Control+Izquierda y Control+Derecha cambian entre "\
      "detalles y eventos.")                                                                       \
    X(KeysPlacing, "Page Down and Page Up go through the spots where it fits and can be used, best first; "\
      "Control with them changes room, Shift turns it on the same spot. Enter buys it there, Escape cancels, "\
      "R repeats. Control+Up and Control+Down read the spot's details and what else was checked.",\
      "Avanzar y Retroceder página recorren los sitios donde cabe y se puede usar, los mejores primero; con "\
      "Control cambian de habitación, con Mayúsculas lo giran en el mismo sitio. Intro lo compra ahí, Escape "\
      "cancela, R repite. Control+Arriba y Control+Abajo leen los detalles del sitio y qué más se comprobó.")\
    X(KeysBuy, "Arrows go through the catalog; a product reads its price and description, Control+Up and "\
      "Control+Down everything "\
      "about it. Enter on a product starts placing it where it fits and can be used. Page Down and Page Up go "\
      "through the lot's objects, Shift with them changes category, Control with them changes room; Enter "\
      "picks the object up to move it, Delete then sells it.",                                     \
      "Las flechas recorren el catálogo; un producto dice su precio y descripción, Control+Arriba y "\
      "Control+Abajo todo sobre él. "\
      "Intro sobre un producto empieza a colocarlo donde cabe y se puede usar. Avanzar y Retroceder página "\
      "recorren los objetos del solar, con Mayúsculas cambian de categoría, con Control de habitación; Intro "\
      "recoge el objeto para moverlo, y Suprimir lo vende.")                                        \
    X(KeysMenu, "Up and down choose an interaction, Enter does it, Escape goes back.",             \
      "Arriba y abajo eligen una interacción, Intro la hace, Escape vuelve atrás.")                \
    X(UnsupportedBuild, "Sims accessibility: unsupported game version, mod disabled",              \
      "Accesibilidad de Los Sims: versión del juego no compatible, mod desactivado")               \
    /* Control state */                                                                            \
    X(Checked, "checked", "marcado")                                                               \
    X(NotChecked, "not checked", "no marcado")                                                     \
    X(Selected, "selected", "seleccionado")                                                        \
    X(TabSelected, "selected", "seleccionada")                                                     \
    X(Unavailable, "unavailable", "no disponible")                                                 \
    X(UnlabeledButton, "unlabeled button", "botón sin nombre")                                     \
    X(Unnamed, "unnamed", "sin nombre")                                                            \
    /* Control roles: what a control is, said after its name */                                    \
    X(RoleButton, "button", "botón")                                                               \
    X(RoleTab, "tab", "pestaña")                                                                   \
    X(RoleCheckBox, "check box", "casilla de verificación")                                        \
    X(RoleSlider, "slider", "control deslizante")                                                  \
    X(RolePicker, "value picker", "selector")                                                      \
    X(NothingHere, "nothing to navigate here", "no hay nada que recorrer aquí")                    \
    X(NextPage, "next page", "página siguiente")                                                   \
    X(BufferDetails, "details", "detalles")                                                        \
    X(CheatConsole, "cheat console", "consola de trucos")                                           \
    X(CheatsUnavailable, "the cheat console is not available here", "la consola de trucos no está disponible aquí")\
    X(CheatRan, "cheat: %s", "truco: %s")                                                          \
    X(CheatOf, "%s, %d of %d", "%s, %d de %d")                                                     \
    X(CheatEmpty, "type or pick a cheat first", "escribe o elige antes un truco")                  \
    X(CheatHint, "Up and Down pick a cheat, typing adds to it, Enter runs it and Enter again repeats it, Escape closes.",\
      "Arriba y Abajo eligen un truco, lo que escribas se añade, Intro lo ejecuta y otro Intro lo repite, Escape cierra.")\
    X(FundsNow, "funds %s, %s", "fondos %s, %s")                                                   \
    X(BufferEvents, "events", "eventos")                                                           \
    X(BufferEmpty, "%s, empty", "%s, vacío")                                                       \
    X(PositionInGroup, "%d of %d, %s", "%d de %d, %s")                                             \
    X(TileAt, "tile %d, %d", "casilla %d, %d")                                                     \
    X(SellsFor, "sells for %s", "se vende por %s")                                                 \
    X(PreviousPage, "previous page", "página anterior")                                            \
    /* Groups */                                                                                   \
    X(GroupSummary, "%s, %d items", "%s, %d elementos")                                            \
    X(GroupSummaryOne, "%s, 1 item", "%s, 1 elemento")                                             \
    X(GroupButtons, "buttons", "botones")                                                          \
    X(GroupText, "text", "texto")                                                                  \
    X(GroupHouses, "houses", "casas")                                                              \
    X(GroupFamilies, "families", "familias")                                                       \
    X(GroupControls, "controls", "controles")                                                      \
    X(GroupOptions, "options", "opciones")                                                         \
    X(GroupCatalog, "catalog", "catálogo")                                                         \
    X(GroupBuild, "build", "construcción")                                                         \
    X(GroupCamera, "camera", "cámara")                                                             \
    X(GroupFamilyPanel, "Sim panels", "paneles del Sim")                                           \
    X(GroupActionQueue, "action queue", "cola de acciones")                                        \
    /* Screens */                                                                                  \
    X(ScreenNeighborhood, "Neighborhood", "Barrio")                                                \
    X(ScreenNeighborhoodNumber, "Neighborhood %s", "Barrio %s")                                    \
    X(ScreenLot, "Lot", "Solar")                                                                   \
    X(ScreenPickFamily, "Choose a family", "Elegir familia")                                       \
    X(ScreenDesignFamily, "Create a family", "Crear familia")                                      \
    X(ScreenDesignCharacter, "Create a Sim", "Crear un Sim")                                       \
    /* Widgets */                                                                                  \
    X(MotiveValue, "%s, %d percent", "%s, %d por ciento")                                          \
    X(QueuedAction, "%s, queued", "%s, en cola")                                                   \
    X(TutorialIcon, "help system tip", "consejo del sistema de ayuda")                             \
    X(HouseNumber, "House %d", "Casa %d")                                                          \
    X(FamilyMemberOne, "1 member", "1 miembro")                                                    \
    X(FamilyMembers, "%d members", "%d miembros")                                                  \
    X(NetWorth, "net worth", "valor neto")                                                         \
    X(Friends, "friends", "amigos")                                                                \
    /* Lot: object browser (PageUp/PageDown) */                                                    \
    X(CategorySims, "Sims", "Sims")                                                                \
    X(CategorySeating, "seating", "asientos")                                                      \
    X(CategorySurfaces, "surfaces", "superficies")                                                 \
    X(CategoryAppliances, "appliances", "electrodomésticos")                                       \
    X(CategoryElectronics, "electronics", "electrónica")                                           \
    X(CategoryPlumbing, "plumbing", "fontanería")                                                  \
    X(CategoryLighting, "lighting", "iluminación")                                                 \
    X(CategoryDecorative, "decorative", "decoración")                                              \
    X(CategoryMiscellaneous, "miscellaneous", "varios")                                            \
    X(CategoryOther, "other objects", "otros objetos")                                             \
    X(NoObjects, "no objects here", "no hay objetos aquí")                                         \
    /* Lot: rooms (Ctrl+PageUp/PageDown) */                                                        \
    X(RoomOutside, "outside", "exterior")                                                          \
    X(RoomBathroom, "bathroom", "baño")                                                            \
    X(RoomBedroom, "bedroom", "dormitorio")                                                        \
    X(RoomKitchen, "kitchen", "cocina")                                                            \
    X(RoomDining, "dining room", "comedor")                                                        \
    X(RoomLiving, "living room", "sala de estar")                                                  \
    X(RoomStudy, "study", "estudio")                                                               \
    X(RoomOther, "room", "habitación")                                                             \
    X(FloorNumber, "floor %d", "piso %d")                                                          \
    X(RoomObjects, "%s, %d objects", "%s, %d objetos")                                             \
    X(RoomObjectsOne, "%s, 1 object", "%s, 1 objeto")                                              \
    X(RoomTiles, "%d tiles", "%d casillas")                                                        \
    X(SameObjects, "%d alike", "%d iguales")                                                       \
    X(NotOnLot, "not on a lot", "no estás en un solar")                                            \
    X(HereNow, "right here", "aquí mismo")                                                         \
    X(TilesAway, "%d tiles %s", "a %d casillas al %s")                                             \
    X(OneTileAway, "1 tile %s", "a 1 casilla al %s")                                               \
    X(DirNorth, "north", "norte")                                                                  \
    X(DirNorthEast, "north-east", "noreste")                                                       \
    X(DirEast, "east", "este")                                                                     \
    X(DirSouthEast, "south-east", "sureste")                                                       \
    X(DirSouth, "south", "sur")                                                                    \
    X(DirSouthWest, "south-west", "suroeste")                                                      \
    X(DirWest, "west", "oeste")                                                                    \
    X(DirNorthWest, "north-west", "noroeste")                                                      \
    X(OtherFloor, "on another floor", "en otro piso")                                              \
    X(SelectedSimSuffix, "selected", "seleccionado")                                               \
    X(PieMenu, "interactions", "interacciones")                                                    \
    X(NoInteractions, "no interactions", "sin interacciones")                                      \
    X(Cancelled, "cancelled", "cancelado")                                                         \
    X(WalkingThere, "walking there", "yendo hacia allí")                                           \
    X(CannotWalk, "cannot walk there", "no puede ir allí")                                         \
    /* Buy mode: catalog */                                                                        \
    X(CannotAfford, "can't afford", "no te alcanza")                                               \
    X(RatingHunger, "hunger", "hambre")                                                            \
    X(RatingComfort, "comfort", "comodidad")                                                       \
    X(RatingHygiene, "hygiene", "higiene")                                                         \
    X(RatingBladder, "bladder", "vejiga")                                                          \
    X(RatingEnergy, "energy", "energía")                                                           \
    X(RatingFun, "fun", "diversión")                                                               \
    X(RatingRoom, "room", "habitación")                                                            \
    X(SkillCooking, "+cooking", "+cocina")                                                         \
    X(SkillMechanical, "+mechanical", "+mecánica")                                                 \
    X(SkillLogic, "+logic", "+lógica")                                                             \
    X(SkillBody, "+body", "+físico")                                                               \
    X(SkillCreativity, "+creativity", "+creatividad")                                              \
    X(SkillCharisma, "+charisma", "+carisma")                                                      \
    X(SkillStudy, "+study", "+estudio")                                                            \
    /* Buy mode: placing */                                                                        \
    X(AreaSpots, "%s, %d spots", "%s, %d sitios")                                                  \
    X(AreaSpotsOne, "%s, 1 spot", "%s, 1 sitio")                                                   \
    X(AreaNoSpot, "%s, no spot: %s", "%s, sin sitio: %s")                                          \
    X(SpotOf, "%d of %d: %s", "%d de %d: %s")                                                      \
    X(NoSpotAnywhere, "no spot on this floor: %s", "no hay sitio en este piso: %s")                \
    X(NoOtherTurn, "no other direction works here", "aquí no cabe en otra dirección")              \
    X(Bought, "bought %s", "comprado: %s")                                                         \
    X(FundsLeft, "%s left", "quedan %s")                                                           \
    X(AnotherInHand, "another one in hand, Escape to stop", "tienes otro en la mano, Escape para terminar")\
    X(PlaceFailed, "could not place it: %s", "no se pudo colocar: %s")                             \
    X(PlacingCancelled, "purchase cancelled", "compra cancelada")                                  \
    X(Moving, "moving %s, sells for %s", "moviendo %s, se vende por %s")                           \
    X(PutDown, "placed: %s", "colocado: %s")                                                       \
    X(PutBack, "%s put back", "%s vuelve a su sitio")                                              \
    X(Sold, "sold %s for %s, you have %s", "vendido %s por %s, tienes %s")                         \
    X(ThrownAway, "%s thrown away", "%s tirado a la basura")                                       \
    X(CannotSell, "it cannot be sold", "no se puede vender")                                       \
    X(CannotPickUp, "cannot pick it up. %s", "no se puede recoger. %s")                            \
    X(AreaChecked, "%s: %d good spots; %d do not fit, %d cannot be reached to use it, %d would block a door, "\
      "%d would leave another object unusable",                                                    \
      "%s: %d sitios buenos; en %d no cabe, en %d no se podría llegar a usarlo, %d bloquearían una puerta, "\
      "%d dejarían otro objeto sin uso")                                                           \
    X(MostlyBecause, "mostly: %s", "sobre todo: %s")                                               \
    X(UsedFromTiles, "used from %d tiles", "se usa desde %d casillas")                                 X(UsedFromTile, "used from 1 tile", "se usa desde 1 casilla")                                  \
    X(RoomEmpty, "empty room, %d tiles", "habitación vacía, %d casillas")                          \
    X(InCorner, "in the %s corner", "en el rincón %s")                                             \
    X(AgainstWall, "back to the %s wall", "contra la pared %s")                                    \
    X(BesideWall, "beside the %s wall", "junto a la pared %s")                                     \
    X(AwayFromWalls, "away from the walls", "lejos de las paredes")                                \
    X(FacingDirection, "facing %s", "mirando al %s")                                               \
    X(FacingObject, "facing %s", "frente a %s")                                                    \
    X(NextToObject, "next to %s", "junto a %s")                                                    \
    X(OnTopOf, "on %s", "sobre %s")                                                                \
    X(MeantFor, "meant for: %s", "para: %s")                                                       \
    X(CutsOff, "cuts off %d tiles", "deja aisladas %d casillas")                                   \
    X(WouldBlockObject, "it would leave %s unusable", "dejaría sin uso %s")                        \
    X(NoReachableSpot, "no Sim could get to it", "ningún Sim podría llegar a usarlo")              \
    X(WouldBlockDoor, "it would block a door", "bloquearía una puerta")                            \
    X(DoesNotFit, "it does not fit", "no cabe")                                                    \
    /* Default lot objects that have no catalog name */                                            \
    X(ObjMailbox, "mailbox", "buzón")                                                              \
    X(ObjTrashOutside, "outdoor trash can", "cubo de basura exterior")                             \
    X(ObjPineTree, "pine tree", "pino")                                                            \
    X(ObjBirchTree, "birch tree", "abedul")                                                        \
    X(ObjSycamoreTree, "sycamore", "sicómoro")                                                     \
    X(ObjOakTree, "oak", "roble")                                                                  \
    X(ObjMapleTree, "maple", "arce")                                                               \
    X(ObjPalmTree, "palm tree", "palmera")                                                         \
    X(ObjWillowTree, "willow", "sauce")                                                            \
    X(ObjAppleTree, "apple tree", "manzano")                                                       \
    X(ObjCherryTree, "cherry tree", "cerezo")                                                      \
    X(ObjElmTree, "elm", "olmo")                                                                   \
    X(ObjCypressTree, "cypress", "ciprés")                                                         \
    X(ObjTree, "tree", "árbol")                                                                    \
    /* Lot: status (the HUD) */                                                                    \
    X(GroupStatus, "status", "estado")                                                             \
    X(GroupFamily, "family", "familia")                                                            \
    X(LabelValue, "%s: %s", "%s: %s")                                                              \
    /* Lot: the selected Sim */                                                                    \
    X(MoodGood, "%s, %d of 5 bars green", "%s, %d de 5 barras en verde")                           \
    X(MoodBad, "%s, %d of 5 bars red", "%s, %d de 5 barras en rojo")                               \
    X(MoodNeutral, "%s, neutral", "%s, neutro")                                                    \
    X(RelationshipValues, "%s. Daily %d, lifetime %d", "%s. Diaria %d, de por vida %d")            \
    X(SkillValue, "%s, %d of 10", "%s, %d de 10")                                                  \
    X(GroupJob, "job", "empleo")                                                                   \
    X(JobPay, "pay", "sueldo")                                                                     \
    X(JobFriendsNeeded, "family friends", "amigos de la familia")                                  \
    X(JobPerformance, "performance", "rendimiento")                                                \
    X(GroupHouse, "house", "casa")                                                                 \
    /* Text fields */                                                                              \
    X(TextField, "%s %s, text field", "%s %s, campo de texto")                                     \
    X(Blank, "blank", "vacío")                                                                     \
    X(Space, "space", "espacio")                                                                   \
    /* Create a Sim */                                                                             \
    X(OutOfTen, "%d of 10", "%d de 10")                                                            \
    X(ValueOfMax, "%d of %d", "%d de %d")                                                          \
    X(PointsLeft, "%d points left", "quedan %d puntos")                                            \
    X(PointLeft, "1 point left", "queda 1 punto")                                                  \
    X(NoPointsLeft, "no points left", "no quedan puntos")                                          \
    X(HeadLook, "head %d of %d: %s", "cabeza %d de %d: %s")                                        \
    X(BodyLook, "body %d of %d, %s: %s", "cuerpo %d de %d, %s: %s")                                \
    X(BodyFit, "fit", "atlético")                                                                  \
    X(BodyFat, "heavy", "grueso")                                                                  \
    X(BodySkinny, "thin", "delgado")                                                               \
    X(BodyLookNoShape, "body %d of %d: %s", "cuerpo %d de %d: %s")                                                               \
    X(SimPreview, "Sim preview", "vista del Sim")                                                  \
    X(GroupPersonality, "personality", "personalidad")                                             \
    X(GroupLooks, "appearance", "aspecto")                                                         \
    /* Family picker */                                                                            \
    X(GroupMembers, "members", "miembros")                                                         \
    X(TraitNeat, "Neat", "Pulcritud")                                                              \
    X(TraitOutgoing, "Outgoing", "Extroversión")                                                   \
    X(TraitActive, "Active", "Actividad")                                                          \
    X(TraitPlayful, "Playful", "Ánimo lúdico")                                                     \
    X(TraitNice, "Nice", "Cordialidad")
