#include "widget/inc/list.h"
#include "BSP/ATK_MD0700/atk_md0700.h"
#include "MALLOC/malloc.h"
#include "gui.h"
#include "log.h"
#include "widget/inc/button.h"

void DrawList(List* list);
void DrawListDefault(List* list);
void drawDividingLineOnCursor(List* list, uint16_t x, uint16_t y);
void drawDividingLine(List* list, uint16_t* pX, uint16_t* pY, uint8_t height);
uint8_t getSubListCntLimit(List* list);

List* NewList(uint16_t xpos,
              uint16_t ypos,
              uint16_t width,
              uint16_t height,
              Border*  border,
              uint16_t headlineHeight,
              uint16_t itemHeight,
              Obj*     scroller,
              uint8_t  dividingLineHeight)
{
    uint8_t cnt    = 0;
    List*   list   = (List*)mymalloc(GUI_MALLOC_SOURCE, sizeof(List));
    list->itemList = (LinkedList*)mymalloc(SRAMEX, sizeof(LinkedList));
    init_LinkedList(list->itemList, NodeDataType_LinkedList);
    list->headItem           = list->itemList->head;
    list->headlineHeight     = headlineHeight;
    list->headlineTextarea   = NULL;
    list->dividingLineHeight = dividingLineHeight;
    list->itemHeight         = itemHeight;
    list->scroller           = *scroller;
    list->border.borderColor = border->borderColor;
    list->border.borderFlag  = border->borderFlag;
    list->border.borderWidth = border->borderWidth;
    ((Obj*)list)->height     = height;
    ((Obj*)list)->width      = width;
    ((Obj*)list)->x          = xpos;
    ((Obj*)list)->y          = ypos;
    ((Obj*)list)->type       = Obj_Type_List;
    list->DrawList           = &DrawList;
    cnt                      = getSubListCntLimit(list);
    LinkedNodeData data;
    init_LinkedList(&data.subList, NodeDataType_Obj);
    while (cnt--) {
        push_tail(list->itemList, &data);
    }
    return list;
}

void DrawList(List* list)
{
    DrawListDefault(list);
}

void DrawListDefault(List* list)
{
    uint16_t    i       = 0;
    uint16_t    x       = ((Obj*)list)->x;
    uint16_t    y       = ((Obj*)list)->y;
    LinkedNode* subList = list->itemList->head;
    drawBorder((Obj*)list, &list->border);
    LinkedNode* node = subList->nodeData.subList.head;
    if (list->headlineTextarea) {
#if RELATIVE_LOCATE_ENABLE
        ((Obj*)list->headlineTextarea)->x += x;
        ((Obj*)list->headlineTextarea)->y += y;
#endif
        if (!checkBoundary(x, y, ((Obj*)list)->width, list->headlineHeight,
                           (Obj*)list->headlineTextarea)) {
            ((Obj*)list->headlineTextarea)->x = x;
            ((Obj*)list->headlineTextarea)->y = y;
            if (((Obj*)list->headlineTextarea)->width > ((Obj*)list)->width) {
                ((Obj*)list->headlineTextarea)->width = ((Obj*)list)->width;
            }
            if (((Obj*)list->headlineTextarea)->height > list->itemHeight) {
                ((Obj*)list->headlineTextarea)->height = list->itemHeight;
            }
        }
        /* Save the background color and restore it after drawing textarea in
         headline */
        COLOR_DATTYPE backColor = GUI_getBackColor();
        GUI_setBackColor(list->headlineTextarea->backColor);
        setPublicAlignType(AlignHorizonalType_CENTER, AlignVerticalType_MIDDLE);
        list->headlineTextarea->DrawTextarea(list->headlineTextarea);
        GUI_setBackColor(backColor);
    }
    drawDividingLine(list, &x, &y, list->headlineHeight);
    setPublicAlignType(AlignHorizonalType_LEFT, AlignVerticalType_MIDDLE);
    while (subList) {
        while (node) {
#if RELATIVE_LOCATE_ENABLE
            node->nodeData.obj->x += x;
            node->nodeData.obj->y += y;
#endif
            if (!checkBoundary(x, y, ((Obj*)list)->width, list->itemHeight,
                               node->nodeData.obj)) {
                /* Reset boundary */
                log_n("%sCheckBoundary fail.", ARROW_STRING);
                log_n("Actual:");
                log_n("x: %d", node->nodeData.obj->x);
                log_n("y: %d", node->nodeData.obj->y);
                log_n("width: %d", node->nodeData.obj->width);
                log_n("height: %d", node->nodeData.obj->height);
                log_n("");
                log_n("Target: ");
                log_n("x: %d", x);
                log_n("y: %d", y);
                log_n("width: %d", ((Obj*)list)->width);
                log_n("height: %d", list->itemHeight);
                log_n("%s", ARROW_STRING);
                node->nodeData.obj->x = x;
                node->nodeData.obj->y = y;
                if (node->nodeData.obj->width > ((Obj*)list)->width) {
                    node->nodeData.obj->width = ((Obj*)list)->width;
                }
                if (node->nodeData.obj->height > list->itemHeight) {
                    node->nodeData.obj->height = list->itemHeight;
                }
            }
            draw_widget(node->nodeData.obj);
            node = node->next;
        }
        drawDividingLine(list, &x, &y, list->itemHeight);
        subList = subList->next;
        if (subList->nodeData.subList.head) {
            node = subList->nodeData.subList.head;
        }
    }

    while (y + list->itemHeight + list->dividingLineHeight <
           (((Obj*)list)->y + ((Obj*)list)->height - 1)) {
        drawDividingLine(list, &x, &y, list->itemHeight);
    }
}

void AppendSubList(List* list)
{
    LinkedNodeData data;
    init_LinkedList(&data.subList, NodeDataType_LinkedList);
    push_tail(list->itemList, &data);
}

void AppendSubListItem(List* list, uint16_t index, Obj* obj)
{
    LinkedNodeData data;
    LinkedNode*    node = get_node(list->itemList, index);
    data.obj            = obj;
    push_tail(&node->nodeData.subList, &data);
}

void drawDividingLineOnCursor(List* list, uint16_t x, uint16_t y)
{
    if (list->dividingLineHeight > 0) {
        uint16_t color = RGB888toRGB565(0x325543);
        atk_md0700_fill(x, y, x + ((Obj*)list)->width - 1,
                        y + list->dividingLineHeight - 1, &color,
                        SINGLE_COLOR_BLOCK);
    }
}

void drawDividingLine(List* list, uint16_t* pX, uint16_t* pY, uint8_t height)
{
    *pY += height;
    if (list->dividingLineHeight > 0) {
        uint16_t color = RGB888toRGB565(0x325543);
        atk_md0700_fill(*pX, *pY, *pX + ((Obj*)list)->width - 1,
                        *pY + list->dividingLineHeight - 1, &color,
                        SINGLE_COLOR_BLOCK);
        *pY += list->dividingLineHeight;
    }
}

uint8_t getSubListCntLimit(List* list)
{
    // uint16_t x      = ((Obj*)list)->x;
    uint16_t y =
        ((Obj*)list)->y + list->headlineHeight + list->dividingLineHeight;
    uint16_t endY = ((Obj*)list)->y + ((Obj*)list)->height - 1;
    // uint16_t width  = ((Obj*)list)->width;
    // uint16_t height = ((Obj*)list)->height;
    uint8_t cnt = 0;
    while ((y + list->itemHeight - 1) <= endY) {
        cnt++;
        y += list->itemHeight;
        y += list->dividingLineHeight;
    }
    return cnt;
}

void SetListHeadlineTextarea(List* list, Textarea* textarea)
{
    list->headlineTextarea = textarea;
    list->DrawList(list);
}

void redrawListItem(List* list)
{
    LinkedNode* lineNode = list->itemList->head;
    LinkedNode* node;
    while (lineNode) {
        node = lineNode->nodeData.subList.head;
        while (node) {
            if (node->nodeData.obj) {
                switch (node->nodeData.obj->type) {
                    case Obj_Type_Button:
                        ((Button*)node->nodeData.obj)
                            ->DrawButton((Button*)node->nodeData.obj);
                        break;
                    default: break;
                }
                node = node->next;
            }
        }
        lineNode = lineNode->next;
    }
}

// 返回子列表，即一行控件，index从0开始
LinkedList* getSubList(List* list, int index)
{
    return &get_node(list->itemList, index)->nodeData.subList;
}
