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

    // Adding a new patient
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

    // Logging of an event
    http.HandleFunc("/log-event", func(w http.ResponseWriter, r *http.Request) {
        err := r.ParseForm()
        if err != nil {
            http.Error(w, err.Error(), http.StatusBadRequest)
            return
        }

        id := r.FormValue("id")
        if len(id) == 0 {
            http.Error(w, "The field `id` was not provided", http.StatusBadRequest)
            return
        }

        msg := r.FormValue("msg")
        if len(msg) == 0 {
            http.Error(w, "The field `msg` was not provided", http.StatusBadRequest)
            return
        }

        query := fmt.Sprintf(`
            INSERT INTO Log (kind, patient_id, message)
            VALUES ("EVENT", "%v", "%v");
        `, id, msg)
        
        _, err = db.Exec(query)
        if err != nil {
            http.Error(w, err.Error(), http.StatusInternalServerError)
            return
        }
    })

    // Start the HTTP server
    err = http.ListenAndServe(port, nil)
    if err != nil {
        log.Fatal(err)
    }
}
