#pragma semicolon 1
#pragma newdecls required
#include <sourcemod>
char g_Data[128]; int g_ShopRevision=7,g_ShopCount=2,g_Prices[2]={100,50},g_Checks;
enum struct TestMenu
{
    int marker;
    void GetItem(int selection, char[] data, int size)
    {
        strcopy(data,size,selection==0 && this.marker==1?g_Data:"");
    }
}
int ShopOfferPrice(int index) { if(index<0||index>=2)ThrowError("out of range quote price access");return g_Prices[index]; }
bool PremiumDigits(const char[] text) { if(!text[0])return false;for(int i;text[i];i++)if(text[i]<'0'||text[i]>'9')return false;return true; }
#define Menu TestMenu
#include "../scripting/premium/shop_quotes.inc"
void Check(bool condition,const char[] label){g_Checks++;if(!condition)ThrowError("FAIL: %s",label);}
public int main()
{
    TestMenu menu;menu.marker=1;int index;
    strcopy(g_Data,sizeof(g_Data),"7:0:100");Check(ShopMenuItem(menu,0,index,true)&&index==0,"confirmed quote binds offer index and price");
    g_Prices[0]=120;Check(!ShopMenuItem(menu,0,index,true),"sale ending cannot charge a higher price from stale confirmation");
    Check(ShopMenuItem(menu,0,index),"opening offer details can refresh an old displayed price");
    g_Prices[0]=80;Check(!ShopMenuItem(menu,0,index,true),"sale starting also requires a fresh confirmation");
    strcopy(g_Data,sizeof(g_Data),"7:1:50");Check(ShopMenuItem(menu,0,index,true)&&index==1,"second offer uses its own confirmed price");
    g_ShopRevision=8;Check(!ShopMenuItem(menu,0,index,true),"catalog reload invalidates old quotes");g_ShopRevision=7;
    strcopy(g_Data,sizeof(g_Data),"7:2:50");Check(!ShopMenuItem(menu,0,index,true),"deleted index never reaches price lookup");
    strcopy(g_Data,sizeof(g_Data),"7:-1:50");Check(!ShopMenuItem(menu,0,index,true),"negative offer index rejected");
    strcopy(g_Data,sizeof(g_Data),"7:0");Check(!ShopMenuItem(menu,0,index,true),"old two-field quote rejected");
    strcopy(g_Data,sizeof(g_Data),"7:0:abc");Check(!ShopMenuItem(menu,0,index,true),"malformed quoted price rejected");
    strcopy(g_Data,sizeof(g_Data),"7:0:80");Check(!ShopMenuItem(menu,1,index,true),"non-offer row cannot confirm purchase");
    PrintToServer("PASS: %d shop quote assertions",g_Checks);return g_Checks;
}
