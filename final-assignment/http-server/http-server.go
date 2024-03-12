package main

import (
    "log"
    "net/http"
)

const port = ":8080"

func main() {
    http.HandleFunc("/add-user", func(w http.ResponseWriter, r *http.Request) {
        log.Println(r.FormValue("name"))
        http.Error(w, "Yes, something went horribly wrong!", http.StatusInternalServerError)
    })

    err := http.ListenAndServe(port, nil)
    if err != nil {
        log.Fatal(err)
    }
}
