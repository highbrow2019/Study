#include <iostream>
#include <vector>
#include <memory>

class ItemStack;
class VendingMachine;
bool RunTestCase(VendingMachine *VendingMachinePtr);
class Item
{
    std::string _title="Empty Stack";
    double _cost=0;
public:
    Item() = default;
    Item(std::string title, double icost) :_title(title), _cost(icost)
    {

    }
    friend ItemStack;
};

class ItemStack
{
    unsigned int _count;
    unsigned short _row;
    unsigned short _col;
    std::shared_ptr<Item> _item;
public:
    ItemStack() = default;
    explicit ItemStack(unsigned short row, unsigned short col) :_row(row), _col(col)
    {
        int count = 0;
        _item=std::make_shared<Item>( Item());
    }
    void placeItem(std::shared_ptr<Item> item,const int &count)
    {
        _item = item;
        _count = count;
    }
    void queryItem()
    {
        std::cout << "Item: "<< _item->_title.c_str()<<std::endl;
        std::cout << "Cost: " << _item->_cost << std::endl;
        std::cout << "Available Quatity: " << _count << std::endl;

    }
    bool retriveItem(double &Amount)
    {
        bool itemRetrived = false;
        if (_count && Amount > _item->_cost)
        {
            Amount -= _item->_cost;
            itemRetrived = true;
        }
        else if (!_count)
        {
            std::cout << "No more Item in stack" << std::endl;
        }
        else
        {
            std::cout << "Insufficent Balance" << std::endl;
        }
        return itemRetrived;
    }
};

class VendingMachine
{
    double totalAmount;
    std::vector< std::vector<ItemStack>> stackMap;
    static const int row;
    static const int col;
    static VendingMachine* Vm;

    VendingMachine() {
        stackMap.resize(row);
        for (int i = 0; i < row; ++i)
        {
            //stackMap.push_back(std::vector<ItemStack>);
            stackMap[i].resize(col);
            for (int j = 0; j < col; ++j)
            {
                stackMap[i][j] = std::move(ItemStack(i, j));
            }
        }
    }
   
public:
    static VendingMachine* getVendingMachine();
    void Refill(int rowid, int colid);
    void Printall();
    void addMoney(double amount)
    {
        totalAmount += amount;
    }
    void returnChange()
    {
        std::cout << "Return amount: " << totalAmount<<std::endl;
        totalAmount = 0;
    }
    void queryforItem(int row, int col)
    {
        stackMap[row][col].queryItem();
    }
    bool retriveItem(const unsigned &row,const unsigned &col)
    {
        return stackMap[row][col].retriveItem(totalAmount);
    }

    friend bool RunTestCase(VendingMachine *VendingMachinePtr);
};
void VendingMachine::Printall()
{
    int i = 0, j = 0;
    for (auto row : stackMap)
    {
        std::cout << std::endl;
        j = 0;
        for (auto data : row)
        {
            std::cout << "Item at pos: " << i << "," << j<<std::endl;
            data.queryItem();
            std::cout << std::endl;
            ++j;
        }
        ++i;
    }

}
/*
void VendingMachine::AddDefaultDataSet()
{
    int i = 0, j = 0;
    std::string itemName[3][5] = {...};
    double cost[3][5] = {...};
    int count[3][5] = {...};
   
    for (auto row : stackMap)
    {
        std::cout << std::endl;
        j = 0;
        for (auto data : row)
        {
            auto itemtoAdd = std::make_shared<Item>(Item(itemName[i][j], cost[i][j]));
            data.placeItem(itemtoAdd,count[i][j]);
            ++j;
        }
        ++i;
    }

}
*/
void VendingMachine::Refill(int rowid, int colid)
{
    char title[20];
    double cost;
    int count;

    std::cout << "Input Item Title" << std::endl;
    std::cin >> title;
    std::cout << "Input Item Cost" << std::endl;
    std::cin >> cost;
    std::cout << "Total Count" << std::endl;
    std::cin >> count;
    std::shared_ptr<Item> item(new Item(static_cast<std::string>(title), cost));
    stackMap[rowid][colid].placeItem(item, count);
}

const int VendingMachine::row = 3;
const int VendingMachine::col = 5;
VendingMachine * VendingMachine::Vm = nullptr;
VendingMachine * VendingMachine::getVendingMachine()
{
    if (Vm)
        return Vm;
    Vm = new VendingMachine();
    return Vm;
}

bool RunTestCase(VendingMachine *VendingMachinePtr)
{
    //Add Item
    /*
        std::string title[3][5] = {"Item1","Item2",...};
        double cost[3][5] = {10,15,20....};
        int count[3][5] = {1,0,2,3,7...};
        //Fill the Array in loop
        std::shared_ptr<Item> item(new Item(static_cast<std::string>(title), cost));
        VendingMachinePtr->stackMap[rowid][colid].placeItem(item, count);
        */
    //
    //    Call diffrent methods required..
    //        And check expected output. if does not match return false;
    //    at end return true if everything matched.
    return true;
}


int main()
{
    VendingMachine *VendingMachinePtr = VendingMachine::getVendingMachine();
    bool machinerunning = true;
    while (machinerunning)
    {
        int opt;

        std::cout << "case 0: Run Test Case:" << std::endl;
        std::cout << "case 1: AddItem:" << std::endl;
        std::cout << "case 2: AddAmount:" << std::endl;
        std::cout << "case 3: retriveItem:" << std::endl;
        std::cout << "case 4: queryforItem" << std::endl;
        std::cout << "case 5: returnChange" << std::endl;
        std::cout << "case 6: print All" << std::endl;
        std::cout << "case 7: stop" << std::endl;
    //    std::cout << "case 8: Default Vending Machine Data" << std::endl;
        std::cin >> opt;

        int row = 0;
        int col = 0;

        switch (opt)
        {
        case 0:
            RunTestCase(VendingMachinePtr);
            break;
        case 1: //Add Item: Only for vendors

            //while (true)
            //{
                //int row = 0;
                //int col = 0;
            //    int Addmore=0;
            std::cout << "provide row and col to update:" << std::endl;
            std::cin >> row;
            std::cin >> col;
            VendingMachinePtr->Refill(row, col);
            //    std::cout << "Want to update more item 0/1: ";
            //    std::cin >> Addmore;
            //    if (!Addmore)
            //        break;
            //}
            break;
        case 2: //Addamount
            std::cout << "Enter Amount: ";
            double amount;
            std::cin >> amount;
            VendingMachinePtr->addMoney(amount);
            break;
        case 3: //retriveItem
            //int row = 0;
            //int col = 0;
            std::cout << "provide row and col to retrive:" << std::endl;
            std::cin >> row;
            std::cin >> col;
            VendingMachinePtr->retriveItem(row, col);
            break;
        case 4: //queryforItem
            //int row = 0;
            //int col = 0;
            std::cout << "provide row and col to queryforItem:" << std::endl;
            std::cin >> row;
            std::cin >> col;
            VendingMachinePtr->queryforItem(row, col);
            break;
        case 5://returnChange
//            int row = 0;
//            int col = 0;
            std::cout << "Returning Amount..." << std::endl;
            //std::cin >> row;
            //std::cin >> col;
            VendingMachinePtr->returnChange();
            break;
        case 6:
            VendingMachinePtr->Printall();
            break;
        case 7://stop
            machinerunning = false;
            break;
        }
    }
    return 0;
}

