# Pulse Monitor {#Pulse_Monitor}

## Oviewview 
Responsible for checking if all clients are alive after a certain time interval.
It spawns a background thread which lists all alive processes in the system and 
compares them with the client list. If a clientPID doesn't exist in the system, 
it is cleaned up. 

Below is the piece of code for the same

```cpp
PulseMonitor::PulseMonitor() {
    Timer* mTimer = new Timer(std::bind(&PulseMonitor::checkForDeadClients, this), true);
    mTimer->startTimer(PULSE_LIMIT);
}
```

```cpp
int8_t PulseMonitor::checkForDeadClients() {
    // stores pid of all the running process right now
    vector<long> processId;
    uint8_t found = 0;
    int8_t status;
    std::vector<int32_t> clientList;
    std::unordered_set<int64_t>* clientHandles;
    Request* request;
    Request* untuneRequest = new Request;
    RequestQueue& requestQueue = RequestQueue::getInstance();

    // This method will internally acquire a shared lock on the table.
    this->mClientDataManager->getActiveClientList(clientList);

    status = getAllRunningProcess(processId);
    if (status != 0)
    {
        return status;
    }

    // Delete the clients if they are dead.
    for(int32_t clientPID: clientList) {
        cout<<"checking for client "<<clientPID<<endl;
        found = 0;
        for(auto &pid: processId) {
            if(pid == (long)clientPID)
            {
                found = 1;
                break;
            }
        }

        // If the client was not found in processId list i.e. currently running process
        // Cleanup the client
        if(!found) {
            // Proceed with further cleanup, for example from: CocoTable.
            clientHandles = this->mClientDataManager->getRequestsByClientID(clientPID);

            std::vector<int64_t> handlesToDelete;
            for(int64_t handle: *clientHandles) {
                handlesToDelete.push_back(handle);
            }

            for(int64_t handle: handlesToDelete) {
                // Untune request deletes from both cocoTable and Request Map
                request = RequestManager::getInstance()->getRequestFromMap(handle);
                untuneRequest->setHandle(handle);
                untuneRequest->setPriority(request->getPriority());
                untuneRequest->setRequestType(REQ_RESOURCE_UNTUNING);
                requestQueue.addAndWakeup(untuneRequest);
            }
            this->mClientDataManager->deleteClientById(clientPID);
        }
    }

    return 0;
}
```