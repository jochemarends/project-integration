package main

import (
    "database/sql"
    "log"
    "fmt"
    "net/http"
    _ "github.com/go-sql-driver/mysql"
)

const port = ":8080"

func main() {
    // Connect to the database
    db, err := sql.Open("mysql", "Jochem:Tilligte@tcp(127.0.0.1)/PI")
    if err != nil {
        log.Fatal(err)
    }
    defer db.Close()

    http.HandleFunc("/add-patient", func(w http.ResponseWriter, r *http.Request) {
        err := r.ParseForm()
        if err != nil {
            http.Error(w, err.Error(), http.StatusBadRequest)
            return
        }

        name := r.FormValue("name")
        if len(name) == 0 {
            http.Error(w, "The field `name` was not provided", http.StatusBadRequest)
            return
        }

        query := fmt.Sprintf(`
            INSERT INTO Patient (name)
            VALUES ("%v");
        `, name)
        
        res, err := db.Exec(query)
        if err != nil {
            http.Error(w, err.Error(), http.StatusInternalServerError)
            return
        }

        id, err := res.LastInsertId()
        if err != nil {
            http.Error(w, err.Error(), http.StatusInternalServerError)
            return
        }

        fmt.Fprint(w, id)
    })

    // Start the HTTP server
    err = http.ListenAndServe(port, nil)
    if err != nil {
        log.Fatal(err)
    }
}
